#include "gfx/renderer.h"
#include "gfx/vk/vertex.h"
#include "gfx/frustum.h"
#include "utils/file_utils.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <glad/vulkan.h>
#include <magic_enum.hpp>

#include <limits>
#include <stdexcept>

namespace inf::gfx {

    static constexpr std::uint32_t SHADOW_MAP_RESOLUTION_X = 4096;
    static constexpr std::uint32_t SHADOW_MAP_RESOLUTION_Y = 4096;
    static constexpr VkExtent2D SHADOW_MAP_EXTENT{ SHADOW_MAP_RESOLUTION_X, SHADOW_MAP_RESOLUTION_Y };
    static constexpr std::uint64_t INSTANCE_DATA_BUFFER_SIZE_INITIAL_BYTES = 4 * 1024 * 1024; // 4MBs

    Renderer::Renderer(Context& context, const Window& window, const Camera& camera, const Timer& timer) :
        context(context), camera(camera), timer(timer), image_index(0), frame_index(0) {
        if (!gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE)) {
            throw std::runtime_error("Failed to load Vulkan function pointers.");
        }
        instance = std::make_unique<vk::Instance>(vk::Instance::create_instance("infinitown"));
        surface = std::make_unique<vk::Surface>(window.create_surface(*instance));
        physical_device = std::make_unique<vk::PhysicalDevice>(vk::Device::choose_optimal_device(*instance, *surface));
        logical_device = std::make_unique<vk::LogicalDevice>(physical_device->create_logical_device(*surface));
        // Reload function pointers to ensure extensions are also loaded after device creation
        if (!gladLoaderLoadVulkan(instance->get_instance(), physical_device->get_physical_device(), logical_device->get_device())) {
            throw std::runtime_error("Failed to reload Vulkan function pointers after device creation.");
        }
        
        memory_allocator = std::make_unique<vk::MemoryAllocator>(vk::MemoryAllocator::create(
            instance->get_instance(), physical_device->get_physical_device(), logical_device->get_device()));
        swap_chain = std::make_unique<vk::SwapChain>(logical_device->create_swap_chain(*surface));

        // Load shaders
        {
            const auto vertex_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/default.vert.bin");
            shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, vertex_shader_bytes));
            
            const auto fragment_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/default.frag.bin");
            shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, fragment_shader_bytes));

            const auto instanced_vs_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/instanced.vert.bin");
            instanced_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, instanced_vs_shader_bytes));
            
            const auto instanced_fs_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/instanced.frag.bin");
            instanced_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, instanced_fs_shader_bytes));

            const auto shadow_map_vs_bytes = utils::FileUtils::read_bytes("assets/shaders/shadow_map.vert.bin");
            shadow_map_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, shadow_map_vs_bytes));
            
            const auto shadow_map_fs_bytes = utils::FileUtils::read_bytes("assets/shaders/shadow_map.frag.bin");
            shadow_map_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, shadow_map_fs_bytes));

            const auto shadow_map_instanced_vs_bytes = utils::FileUtils::read_bytes("assets/shaders/instanced_shadow_map.vert.bin");
            shadow_map_instanced_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, shadow_map_instanced_vs_bytes));
            shadow_map_instanced_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, shadow_map_fs_bytes));

            const auto rain_shader_vs_bytes = utils::FileUtils::read_bytes("assets/shaders/rain.vert.bin");
            const auto rain_shader_fs_bytes = utils::FileUtils::read_bytes("assets/shaders/rain.frag.bin");
            particle_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, rain_shader_vs_bytes));
            particle_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, rain_shader_fs_bytes));
        }

        // Create descriptor pool and set layouts for shader uniform data
        descriptor_pool = std::make_unique<vk::DescriptorPool>(vk::DescriptorPool::create(logical_device.get(), 10));
        descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            }));
        instanced_descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            }));
        shadow_map_descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            }
        ));
        particle_descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
            }
        ));

        // Create render pass and graphics pipeline
        const auto& swap_chain_extent = swap_chain->get_extent();
        const auto is_msaa_4x_supported = physical_device->is_sample_count_supported(VK_SAMPLE_COUNT_4_BIT);
        // TODO: MSAA messes up the SRGB color space for some reason, possibly an AMD driver bug
        const auto sample_count = is_msaa_4x_supported ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;
        render_pass = std::make_unique<vk::RenderPass>(vk::RenderPass::create_render_pass(
            logical_device.get(), swap_chain->get_format(), sample_count));
        shadow_map_render_pass = std::make_unique<vk::RenderPass>(vk::RenderPass::create_shadow_render_pass(logical_device.get()));

        // Create default render pipeline
        const auto default_binding_description = vk::Vertex::get_default_binding_description();
        const auto default_attribute_descriptions = vk::Vertex::get_default_attribute_descriptions();
        pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(),
            *render_pass,
            swap_chain->get_extent(),
            *descriptor_set_layout,
            shaders,
            1, &default_binding_description,
            static_cast<std::uint32_t>(default_attribute_descriptions.size()), default_attribute_descriptions.data(),
            sample_count,
            std::nullopt));

        // Create instanced render pipeline
        const auto instanced_binding_descriptions = vk::Vertex::get_instanced_binding_descriptions();
        const auto instanced_attribute_descriptions = vk::Vertex::get_instanced_attribute_descriptions();
        instanced_pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(),
            *render_pass,
            swap_chain->get_extent(),
            *instanced_descriptor_set_layout,
            instanced_shaders,
            static_cast<std::uint32_t>(instanced_binding_descriptions.size()), instanced_binding_descriptions.data(),
            static_cast<std::uint32_t>(instanced_attribute_descriptions.size()), instanced_attribute_descriptions.data(),
            sample_count,
            std::nullopt));
        
        // Create shadow map pipeline (uses default vertex bindings and attributes)
        const auto shadow_map_depth_bias = gfx::vk::PipelineDepthBias{ 1.8f, 2.5f };
        shadow_map_pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(),
            *shadow_map_render_pass,
            SHADOW_MAP_EXTENT,
            *shadow_map_descriptor_set_layout,
            shadow_map_shaders,
            1, &default_binding_description,
            static_cast<std::uint32_t>(default_attribute_descriptions.size()), default_attribute_descriptions.data(),
            VK_SAMPLE_COUNT_1_BIT,
            shadow_map_depth_bias));

        shadow_map_instanced_pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(),
            *shadow_map_render_pass,
            SHADOW_MAP_EXTENT,
            *shadow_map_descriptor_set_layout,
            shadow_map_instanced_shaders,
            static_cast<std::uint32_t>(instanced_binding_descriptions.size()), instanced_binding_descriptions.data(),
            static_cast<std::uint32_t>(instanced_attribute_descriptions.size()), instanced_attribute_descriptions.data(),
            VK_SAMPLE_COUNT_1_BIT,
            shadow_map_depth_bias));

        // Create pipeline for rain effects
        std::array<VkVertexInputBindingDescription, 2> particle_binding_descriptions;
        particle_binding_descriptions[0].binding = 0;
        particle_binding_descriptions[0].stride = sizeof(glm::vec3);
        particle_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        particle_binding_descriptions[1].binding = 1;
        particle_binding_descriptions[1].stride = sizeof(glm::vec3);
        particle_binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        std::array<VkVertexInputAttributeDescription, 2> particle_attribute_descriptions;
        particle_attribute_descriptions[0].binding = 0;
        particle_attribute_descriptions[0].location = 0;
        particle_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        particle_attribute_descriptions[0].offset = 0;
        particle_attribute_descriptions[1].binding = 1;
        particle_attribute_descriptions[1].location = 1;
        particle_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        particle_attribute_descriptions[1].offset = 0;

        particle_pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(),
            *render_pass,
            swap_chain_extent,
            *particle_descriptor_set_layout,
            particle_shaders,
            static_cast<std::uint32_t>(particle_binding_descriptions.size()), particle_binding_descriptions.data(),
            static_cast<std::uint32_t>(particle_attribute_descriptions.size()), particle_attribute_descriptions.data(),
            sample_count,
            std::nullopt));

        // Create a separate color image if necessary because of multisampling
        // If not necessary (sample count = 1), we use the swapchain image instead.
        if (sample_count > VK_SAMPLE_COUNT_1_BIT) {
            color_image = std::make_unique<vk::Image>(vk::Image::create(
                memory_allocator.get(),
                swap_chain_extent.width,
                swap_chain_extent.height,
                swap_chain->get_format(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                sample_count));
            
            color_image_view = std::make_unique<vk::ImageView>(vk::ImageView::create(
                logical_device.get(), color_image->get_image(), swap_chain->get_format(), VK_IMAGE_ASPECT_COLOR_BIT));
        }

        // Create depth buffer
        depth_buffer = std::make_unique<vk::DepthBuffer>(vk::DepthBuffer::create(
            logical_device.get(), memory_allocator.get(), swap_chain_extent, sample_count, false));
        const auto& depth_image_view = depth_buffer->get_image_view();

        // Create a separate depth buffer for the shadow map that will be sampled during the second render pass
        shadow_map_depth_buffer = std::make_unique<vk::DepthBuffer>(vk::DepthBuffer::create(
            logical_device.get(), memory_allocator.get(), SHADOW_MAP_EXTENT, VK_SAMPLE_COUNT_1_BIT, true));

        // Create framebuffers for swapchain images
        for (const auto& image_view : swap_chain->get_image_views()) {
            framebuffers.emplace_back(vk::Framebuffer::create_from_image_view(
                logical_device.get(), *render_pass, image_view, depth_image_view, color_image_view.get(), swap_chain_extent));
        }

        // Create framebuffers for the shadow map
        shadow_map_framebuffer = std::make_unique<vk::Framebuffer>(vk::Framebuffer::create_for_shadow_map(
            logical_device.get(), *shadow_map_render_pass, shadow_map_depth_buffer->get_image_view(), SHADOW_MAP_EXTENT));

        // Create a sampler for the shadow map
        shadow_map_sampler = std::make_unique<vk::Sampler>(vk::Sampler::create(logical_device.get()));

        // Create a command pool, allocate command buffers and semaphores/fences required for swapping framebuffers
        // Finally create the mapped memory regions required to upload uniform buffer data.
        command_pool = std::make_unique<vk::CommandPool>(vk::CommandPool::create_command_pool(logical_device.get(), physical_device->get_queue_family_indices()));
        for (std::uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            command_buffers.emplace_back(command_pool->allocate_buffer());
            image_available_semaphores.emplace_back(vk::Semaphore::create(logical_device.get()));
            render_finished_semaphores.emplace_back(vk::Semaphore::create(logical_device.get()));
            in_flight_fences.emplace_back(vk::Fence::create(logical_device.get(), true));
            uniform_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::UNIFORM_BUFFER, sizeof(Matrices)));
            instanced_data_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::VERTEX_BUFFER, INSTANCE_DATA_BUFFER_SIZE_INITIAL_BYTES));
            instanced_shadow_data_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::VERTEX_BUFFER, INSTANCE_DATA_BUFFER_SIZE_INITIAL_BYTES));
            particle_data_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::VERTEX_BUFFER, INSTANCE_DATA_BUFFER_SIZE_INITIAL_BYTES));
            shadow_map_uniform_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::UNIFORM_BUFFER, sizeof(Matrices)));
            particle_uniform_buffers.emplace_back(vk::MappedBuffer::create(
                logical_device.get(), memory_allocator.get(), vk::BufferType::UNIFORM_BUFFER, sizeof(ParticleMatrices)));
        }

        // Allocate descriptor sets for the uniform buffers and the shadow map sampler
        std::vector<VkBuffer> uniform_buffer_handles(uniform_buffers.size());
        std::vector<VkDescriptorBufferInfo> buffer_infos(uniform_buffers.size());
        std::vector<VkDescriptorImageInfo> image_infos(uniform_buffers.size());
        std::vector<std::vector<VkWriteDescriptorSet>> write_descriptor_sets(uniform_buffers.size());
        for (std::size_t i = 0; i < uniform_buffers.size(); ++i) {
            // Push the uniform buffer to the write descriptor set
            uniform_buffer_handles[i] = uniform_buffers[i].get_buffer();
            auto& buffer_info = buffer_infos[i];
            buffer_info = {};
            buffer_info.buffer = uniform_buffer_handles[i];
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE;
            write_descriptor_sets[i].emplace_back(gfx::vk::WriteDescriptorSet::create_for_buffer(buffer_info, 0));
            // Push the sampler to the write descriptor set
            auto& image_info = image_infos[i];
            image_info = {};
            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            image_info.imageView = shadow_map_depth_buffer->get_image_view().get_image_view();
            image_info.sampler = shadow_map_sampler->get_sampler();
            write_descriptor_sets[i].emplace_back(gfx::vk::WriteDescriptorSet::create_for_sampler(image_info, 1));
        }
        descriptor_sets = descriptor_pool->allocate_sets(
            *descriptor_set_layout, write_descriptor_sets, static_cast<std::uint32_t>(uniform_buffer_handles.size()));

        std::vector<VkDescriptorBufferInfo> shadow_map_buffer_infos(shadow_map_uniform_buffers.size());
        std::vector<std::vector<VkWriteDescriptorSet>> shadow_map_write_descriptor_sets(shadow_map_uniform_buffers.size());
        for (std::size_t i = 0; i < shadow_map_uniform_buffers.size(); ++i) {
            auto& shadow_map_buffer_info = shadow_map_buffer_infos[i];
            shadow_map_buffer_info = {};
            shadow_map_buffer_info.buffer = shadow_map_uniform_buffers[i].get_buffer();
            shadow_map_buffer_info.offset = 0;
            shadow_map_buffer_info.range = VK_WHOLE_SIZE;
            shadow_map_write_descriptor_sets[i].emplace_back(gfx::vk::WriteDescriptorSet::create_for_buffer(shadow_map_buffer_info, 0));
        }
        shadow_map_descriptor_sets = descriptor_pool->allocate_sets(
            *shadow_map_descriptor_set_layout, shadow_map_write_descriptor_sets, static_cast<std::uint32_t>(shadow_map_uniform_buffers.size()));

        std::vector<VkDescriptorBufferInfo> particle_buffer_infos(particle_uniform_buffers.size());
        std::vector<std::vector<VkWriteDescriptorSet>> particle_write_descriptor_sets(particle_uniform_buffers.size());
        for (std::size_t i = 0; i < particle_uniform_buffers.size(); ++i) {
            auto& particle_buffer_info = particle_buffer_infos[i];
            particle_buffer_info = {};
            particle_buffer_info.buffer = particle_uniform_buffers[i].get_buffer();
            particle_buffer_info.offset = 0;
            particle_buffer_info.range = VK_WHOLE_SIZE;
            particle_write_descriptor_sets[i].emplace_back(gfx::vk::WriteDescriptorSet::create_for_buffer(particle_buffer_info, 0));
        }
        particle_descriptor_sets = descriptor_pool->allocate_sets(
            *particle_descriptor_set_layout, particle_write_descriptor_sets, static_cast<std::uint32_t>(particle_uniform_buffers.size()));

        projection_matrix = glm::perspective(
            FOVY,
            static_cast<float>(swap_chain_extent.width) / swap_chain_extent.height,
            NEAR_PLANE,
            FAR_PLANE);
        // Flip the Y axis to account for the degenerate coordinate system of Vulkan
        projection_matrix[1][1] *= -1.0f;

        init_imgui(window, sample_count);
    }

    const Camera& Renderer::get_camera() const {
        return camera;
    }

    const vk::Instance& Renderer::get_vulkan_instance() const {
        return *instance;
    }

    const vk::PhysicalDevice& Renderer::get_physical_device() const {
        return *physical_device;
    }

    const vk::LogicalDevice& Renderer::get_logical_device() const {
        return *logical_device;
    }

    const vk::MemoryAllocator& Renderer::get_memory_allocator() const {
        return *memory_allocator;
    }

    void Renderer::begin_frame(
        Weather world_weather,
        RainIntensity world_rain_intensity,
        std::size_t num_districts,
        std::size_t num_buildings) {
        shadow_casters_to_render.clear();
        instanced_non_casters_to_render.clear();
        instanced_casters_to_render.clear();
        bounding_boxes_to_render.clear();
        particles_to_render.clear();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (context.show_diagnostics) {
            ImGui::Begin("Diagnostics");
            ImVec2 window_size(400, 350);

            // Performance data
            ImGui::Text("FPS: %d", timer.get_fps());
            ImGui::Text("Districts: %d", static_cast<int>(num_districts));
            ImGui::Text("Buildings: %d", static_cast<int>(num_buildings));

            // Camera data
            const auto format_vec3 = [](const glm::vec3& vec) {
                return "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "]";
            };
            const auto position = format_vec3(camera.get_position());
            const auto direction = format_vec3(camera.get_direction());
            ImGui::Separator();
            ImGui::Text("Camera position: %s", position.c_str());
            ImGui::Text("Camera direction: %s", direction.c_str());
            ImGui::SliderFloat("Camera speed", &context.camera_speed, Context::CAMERA_SPEED_MIN, Context::CAMERA_SPEED_MAX);

            // Time of day
            ImGui::Separator();
            ImGui::Checkbox("Fix time of day", &context.fix_time_of_day);
            ImGui::SliderFloat("Time of day", &context.time_of_day, 0.0f, 1.0f);
            ImGui::Text("Light factor: %f", glm::sin(context.time_of_day * glm::pi<float>()));
            ImGui::Separator();

            // Weather
            if (ImGui::Checkbox("Override weather", &context.override_weather)) {
                context.force_weather_change(world_weather, world_rain_intensity);
            }
            if (context.override_weather) {
                // Display weather combo box
                const auto context_weather = context.get_overriden_weather();
                std::string weather_str(magic_enum::enum_name(context_weather));
                if (ImGui::BeginCombo("Weather", weather_str.c_str())) {
                    for (std::size_t i = 0; i < magic_enum::enum_count<Weather>(); ++i) {
                        const auto enum_member = magic_enum::enum_value<Weather>(i);
                        std::string enum_name(magic_enum::enum_name(enum_member));
                        bool is_selected = enum_member == context_weather;
                        if (ImGui::Selectable(enum_name.c_str(), is_selected)) {
                            RainIntensity rain_intensity;
                            if (enum_member == Weather::SUNNY) {
                                rain_intensity = RainIntensity::NONE;
                            }
                            else if (context.get_overriden_rain_intensity() == RainIntensity::NONE) {
                                rain_intensity = RainIntensity::LIGHT;
                            }
                            else {
                                rain_intensity = context.get_overriden_rain_intensity();
                            }
                            context.force_weather_change(enum_member, rain_intensity);
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                // Display rain intensity combo box if the overriden weather is set to rainy
                if (context_weather == Weather::RAINY) {
                    const auto context_rain_intensity = context.get_overriden_rain_intensity();
                    std::string rain_intensity_str(magic_enum::enum_name(context_rain_intensity));
                    if (ImGui::BeginCombo("Rain intensity", rain_intensity_str.c_str())) {
                        for (std::size_t i = 0; i < magic_enum::enum_count<RainIntensity>(); ++i) {
                            const auto enum_member = magic_enum::enum_value<RainIntensity>(i);
                            if (enum_member == RainIntensity::NONE) {
                                continue;
                            }
                            std::string enum_name(magic_enum::enum_name(enum_member));
                            bool is_selected = enum_member == context_rain_intensity;
                            if (ImGui::Selectable(enum_name.c_str(), is_selected)) {
                                context.force_weather_change(context.get_overriden_weather(), enum_member);
                            }
                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
            }
            else {
                ImGui::SliderFloat(
                    "Change frequency",
                    &context.weather_change_frequency_seconds,
                    Context::WEATHER_CHANGE_FREQUENCY_SECONDS_MIN,
                    Context::WEATHER_CHANGE_FREQUENCY_SECONDS_MAX,
                    "%.3f seconds");
                ImGui::SliderFloat(
                    "Change chance",
                    &context.weather_change_chance_percentage,
                    Context::WEATHER_CHANGE_CHANCE_PERCENTAGE_MIN,
                    Context::WEATHER_CHANGE_CHANCE_PERCENTAGE_MAX,
                    "%.3f%");
            }
            const std::string weather_str(magic_enum::enum_name(context.override_weather
                ? context.get_overriden_weather()
                : world_weather));
            ImGui::Text("Weather: %s", weather_str.c_str());
            const std::string rain_intensity_str(magic_enum::enum_name(context.override_weather
                ? context.get_overriden_rain_intensity()
                : world_rain_intensity));
            ImGui::Text("Rain intensity: %s", rain_intensity_str.c_str());

            // Development features
            ImGui::Separator();
            ImGui::Checkbox("Show debug BBs", &context.show_debug_bbs);
            ImGui::SetWindowSize({ window_size.x, window_size.y });
            ImGui::End();
        }
    }

    void Renderer::render(const Mesh& mesh) {
        shadow_casters_to_render.emplace_back(&mesh);
    }

    void Renderer::render_instanced(const Mesh& mesh, const std::vector<glm::vec3>& positions, const std::vector<float>& rotations) {
        if (positions.empty()) {
            return;
        }
        instanced_non_casters_to_render.emplace_back(InstancedMeshToRender{ &mesh, positions, rotations });
    }

    void Renderer::render_instanced_caster(const Mesh& mesh, const std::vector<glm::vec3>& positions, const std::vector<float>& rotations) {
        if (positions.empty()) {
            return;
        }
        instanced_casters_to_render.emplace_back(InstancedMeshToRender{ &mesh, positions, rotations });
    }

    void Renderer::render_particles(const Mesh& mesh, const std::vector<glm::vec3>& positions) {
        if (positions.empty()) {
            return;
        }
        particles_to_render.emplace_back(ParticlesToRender{ &mesh, positions });
    }

    void Renderer::render(const BoundingBox3D& bounding_box, const glm::vec3& color) {
        if (!context.show_debug_bbs) {
            return;
        }
        const auto& bb = bounding_box;
        const auto vertices = bb.to_vertices(0.002f, color);
        const auto num_bytes = vertices.size() * sizeof(vk::Vertex);
        const auto& buffer = bounding_boxes_to_render.emplace_back(
            vk::MappedBuffer::create(logical_device.get(), memory_allocator.get(), vk::BufferType::VERTEX_BUFFER, num_bytes));
        buffer.upload(vertices.data(), num_bytes);
    }

    void Renderer::end_frame() {
        // Wait for the previous frame to finish
        in_flight_fences[frame_index].wait_for_and_reset();

        // Acquire the next image in the swap chain
        vkAcquireNextImageKHR(
            logical_device->get_device(),
            swap_chain->get_swap_chain(),
            std::numeric_limits<std::uint64_t>::max(),
            image_available_semaphores[frame_index].get_semaphore(),
            VK_NULL_HANDLE,
            &image_index);

        const auto& command_buffer = command_buffers[frame_index];
        command_buffer.reset();
        command_buffer.begin();

        // In the first render pass we render into a shadow map which will be sampled in the second render pass
        std::vector<VkClearValue> shadow_map_clear_values(1);
        shadow_map_clear_values[0].depthStencil = { 1.0f, 0 };
        shadow_map_render_pass->begin(*shadow_map_framebuffer, SHADOW_MAP_EXTENT, command_buffer, shadow_map_clear_values);
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_map_pipeline->get_pipeline());
        // Since the viewport and the scissor is dynamic we need to supply it each frame
        VkViewport shadow_map_viewport{};
        shadow_map_viewport.x = 0.0f;
        shadow_map_viewport.y = 0.0f;
        shadow_map_viewport.width = static_cast<float>(SHADOW_MAP_EXTENT.width);
        shadow_map_viewport.height = static_cast<float>(SHADOW_MAP_EXTENT.height);
        shadow_map_viewport.minDepth = 0.0f;
        shadow_map_viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer.get_command_buffer(), 0, 1, &shadow_map_viewport);

        VkRect2D shadow_map_scissor{};
        shadow_map_scissor.offset = { 0, 0 };
        shadow_map_scissor.extent = SHADOW_MAP_EXTENT;
        vkCmdSetScissor(command_buffer.get_command_buffer(), 0, 1, &shadow_map_scissor);

        // Bind descriptor sets
        vkCmdBindDescriptorSets(
            command_buffer.get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            shadow_map_pipeline->get_pipeline_layout(),
            0, 1,
            &shadow_map_descriptor_sets[frame_index],
            0, nullptr);

        const auto view_matrix = camera.to_view_matrix();
        Frustum frustum(projection_matrix * view_matrix);
        // TODO: The number of splits necessary is mostly resolution dependant. 5 splits seem good enough on a 1440p resolution, while
        // even 2 splits seem mostly okay on a fullHD screen. If there is time there should be a proper cascaded shadow implementation
        // instead of this hardcoded value. Or as a janky alternative the engine could dynamically switch to higher splits on higher
        // resolutions, but that would significantly reduce shadow distance on high resolution displays.
        frustum = frustum.split<5>()[0];
        const auto frustum_bb = frustum.compute_bounding_box();
        const auto camera_position = camera.get_position();
        const auto initial_sun_position = glm::vec3(
            camera_position.x + 10.0f,
            camera_position.y + 5.0f,
            frustum_bb.max.z);
        const auto end_sun_position = glm::vec3(
            camera_position.x - 10.0f,
            camera_position.y + 5.0f,
            frustum_bb.max.z);
        const auto sun_position = glm::mix(initial_sun_position, end_sun_position, context.time_of_day);
        static constexpr glm::vec3 sun_start_direction(-0.65f, -0.54f, -0.54f);
        static constexpr glm::vec3 sun_end_direction(0.65f, -0.54f, -0.54f);
        const auto sun_direction = glm::normalize(glm::mix(sun_start_direction, sun_end_direction, context.time_of_day));
        const auto sin_time_of_day = glm::sin(context.time_of_day * glm::pi<float>());
        const auto ambient_light = glm::mix(0.05f, 1.0f, sin_time_of_day);
        glm::mat4 sun_view_matrix = glm::lookAt(
            sun_position,
            sun_position + sun_direction,
            glm::vec3(0.0f, 1.0f, 0.0f));
        BoundingBox3D shadow_bb;
        for (const auto& point : frustum.points) {
            const auto transformed = sun_view_matrix * glm::vec4(point, 1.0f);
            shadow_bb.update(transformed / transformed.w);
        }

        // Upload uniform buffer data
        Matrices shadow_map_matrices;
        glm::mat4 shadow_map_projection_matrix = glm::ortho(
            shadow_bb.min.x,
            shadow_bb.max.x,
            shadow_bb.min.y,
            shadow_bb.max.y,
            NEAR_PLANE,
            FAR_PLANE);
        shadow_map_matrices.projection_matrix = shadow_map_projection_matrix;
        shadow_map_matrices.view_matrix = sun_view_matrix;
        shadow_map_matrices.light_space_matrix = glm::mat4(1.0f);
        shadow_map_matrices.ambient_light = ambient_light;
        shadow_map_matrices.light_direction = sun_direction;
        shadow_map_uniform_buffers[frame_index].upload(&shadow_map_matrices, sizeof(Matrices));

        const auto command_buffer_handle = command_buffer.get_command_buffer();
        for (const auto& mesh : shadow_casters_to_render) {
            // Push model matrix
            PushConstants constants{ mesh->get_model_matrix(), 0 };
            vkCmdPushConstants(
                command_buffer_handle,
                shadow_map_pipeline->get_pipeline_layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(PushConstants),
                &constants);
            static const VkDeviceSize offset = 0;
            const auto buffer_handle = mesh->get_buffer().get_buffer();
            vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &buffer_handle, &offset);
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(mesh->get_number_of_vertices()), 1, 0, 0);
        }

        // Render instanced shadow casters
        {
            vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_map_instanced_pipeline->get_pipeline());
            vkCmdBindDescriptorSets(
                command_buffer.get_command_buffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                shadow_map_instanced_pipeline->get_pipeline_layout(),
                0, 1,
                &shadow_map_descriptor_sets[frame_index],
                0, nullptr);

            // Calculate how many bytes are needed for instance data
            std::uint32_t instanced_data_buffer_bytes = 0;
            std::vector<VkDeviceSize> instanced_data_offsets;
            VkDeviceSize offset_accumulator = 0;
            for (const auto& entry : instanced_casters_to_render) {
                const auto num_bytes = entry.positions.size() * sizeof(glm::vec3) + entry.rotations.size() * sizeof(float);
                instanced_data_buffer_bytes += static_cast<std::uint32_t>(num_bytes);
                instanced_data_offsets.emplace_back(offset_accumulator);
                offset_accumulator += num_bytes;
            }

            // Create a single buffer that will be offset for different meshes
            std::vector<float> data_to_upload;
            data_to_upload.reserve(instanced_data_buffer_bytes / sizeof(float));
            for (const auto& entry : instanced_casters_to_render) {
                for (std::size_t i = 0; i < entry.positions.size(); ++i) {
                    data_to_upload.emplace_back(entry.positions[i].x);
                    data_to_upload.emplace_back(entry.positions[i].y);
                    data_to_upload.emplace_back(entry.positions[i].z);
                    data_to_upload.emplace_back(entry.rotations[i]);
                }
            }
            instanced_shadow_data_buffers[frame_index].upload(data_to_upload.data(), instanced_data_buffer_bytes);

            // Render instanced meshes
            for (std::size_t i = 0; i < instanced_casters_to_render.size(); ++i) {
                const auto& entry = instanced_casters_to_render[i];
                const auto instance_count = static_cast<std::uint32_t>(entry.positions.size());
                std::array<VkDeviceSize, 2> offsets{ 0, instanced_data_offsets.at(i) };
                std::array<VkBuffer, 2> buffer_handles{
                    entry.mesh->get_buffer().get_buffer(),
                    instanced_shadow_data_buffers[frame_index].get_buffer()
                };
                vkCmdBindVertexBuffers(command_buffer_handle, 0, static_cast<std::uint32_t>(buffer_handles.size()), buffer_handles.data(), offsets.data());
                vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(entry.mesh->get_number_of_vertices()), instance_count, 0, 0);
            }
        }

        shadow_map_render_pass->end(command_buffer);

        // In the second render pass we render color data
        const auto& extent = swap_chain->get_extent();
        std::vector<VkClearValue> clear_values(2);
        static constexpr glm::vec4 start_clear_color(0.670588f, 0.87843f, 1.0f, 1.0f);
        static constexpr glm::vec4 end_clear_color(0.0156862f, 0.129412f, 0.2f, 1.0f);
        const auto clear_color = glm::mix(end_clear_color, start_clear_color, sin_time_of_day);
        clear_values[0].color = {{ clear_color.x, clear_color.y, clear_color.z, clear_color.w }};
        clear_values[1].depthStencil = { 1.0f, 0 };
        render_pass->begin(framebuffers[image_index], extent, command_buffer, clear_values);
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer.get_command_buffer(), 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(command_buffer.get_command_buffer(), 0, 1, &scissor);
        vkCmdBindDescriptorSets(
            command_buffer.get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->get_pipeline_layout(),
            0, 1,
            &descriptor_sets[frame_index],
            0, nullptr);

        // Upload uniform buffer data
        Matrices matrices;
        matrices.projection_matrix = projection_matrix;
        matrices.view_matrix = view_matrix;
        matrices.light_space_matrix = shadow_map_projection_matrix * sun_view_matrix;
        matrices.light_direction = sun_direction;
        matrices.ambient_light = ambient_light;
        uniform_buffers[frame_index].upload(&matrices, sizeof(Matrices));

        // Render the meshes
        for (const auto& mesh : shadow_casters_to_render) {
            // Push model matrix
            PushConstants constants{ mesh->get_model_matrix(), 0 };
            vkCmdPushConstants(
                command_buffer_handle,
                pipeline->get_pipeline_layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(PushConstants),
                &constants);

            static const VkDeviceSize offset = 0;
            const auto buffer_handle = mesh->get_buffer().get_buffer();
            vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &buffer_handle, &offset);
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(mesh->get_number_of_vertices()), 1, 0, 0);
        }

        // Render instanced data
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, instanced_pipeline->get_pipeline());
        vkCmdBindDescriptorSets(
            command_buffer.get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            instanced_pipeline->get_pipeline_layout(),
            0, 1,
            &descriptor_sets[frame_index],
            0, nullptr);

        // Calculate how many bytes are needed for instance data
        std::uint32_t instanced_data_buffer_bytes = 0;
        std::vector<VkDeviceSize> instanced_data_offsets;
        VkDeviceSize offset_accumulator = 0;
        for (const auto& entry : instanced_non_casters_to_render) {
            const auto num_bytes = entry.positions.size() * sizeof(glm::vec3) + entry.rotations.size() * sizeof(float);
            instanced_data_buffer_bytes += static_cast<std::uint32_t>(num_bytes);
            instanced_data_offsets.emplace_back(offset_accumulator);
            offset_accumulator += num_bytes;
        }
        for (const auto& entry : instanced_casters_to_render) {
            const auto num_bytes = entry.positions.size() * sizeof(glm::vec3) + entry.rotations.size() * sizeof(float);
            instanced_data_buffer_bytes += static_cast<std::uint32_t>(num_bytes);
            instanced_data_offsets.emplace_back(offset_accumulator);
            offset_accumulator += num_bytes;
        }

        // Create a single buffer that will be offset for different meshes
        std::vector<float> data_to_upload;
        data_to_upload.reserve(instanced_data_buffer_bytes / sizeof(float));
        for (const auto& entry : instanced_non_casters_to_render) {
            for (std::size_t i = 0; i < entry.positions.size(); ++i) {
                data_to_upload.emplace_back(entry.positions[i].x);
                data_to_upload.emplace_back(entry.positions[i].y);
                data_to_upload.emplace_back(entry.positions[i].z);
                data_to_upload.emplace_back(entry.rotations[i]);
            }
        }
        for (const auto& entry : instanced_casters_to_render) {
            for (std::size_t i = 0; i < entry.positions.size(); ++i) {
                data_to_upload.emplace_back(entry.positions[i].x);
                data_to_upload.emplace_back(entry.positions[i].y);
                data_to_upload.emplace_back(entry.positions[i].z);
                data_to_upload.emplace_back(entry.rotations[i]);
            }
        }
        instanced_data_buffers[frame_index].upload(data_to_upload.data(), instanced_data_buffer_bytes);

        // Render instanced meshes
        for (std::size_t i = 0; i < instanced_non_casters_to_render.size(); ++i) {
            const auto& entry = instanced_non_casters_to_render[i];
            const auto instance_count = static_cast<std::uint32_t>(entry.positions.size());
            std::array<VkDeviceSize, 2> offsets{ 0, instanced_data_offsets.at(i) };
            std::array<VkBuffer, 2> buffer_handles{
                entry.mesh->get_buffer().get_buffer(),
                instanced_data_buffers[frame_index].get_buffer()
            };
            vkCmdBindVertexBuffers(command_buffer_handle, 0, static_cast<std::uint32_t>(buffer_handles.size()), buffer_handles.data(), offsets.data());
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(entry.mesh->get_number_of_vertices()), instance_count, 0, 0);
        }
        for (std::size_t i = 0; i < instanced_casters_to_render.size(); ++i) {
            const auto& entry = instanced_casters_to_render[i];
            const auto instance_count = static_cast<std::uint32_t>(entry.positions.size());
            std::array<VkDeviceSize, 2> offsets{ 0, instanced_data_offsets.at(instanced_non_casters_to_render.size() + i) };
            std::array<VkBuffer, 2> buffer_handles{
                entry.mesh->get_buffer().get_buffer(),
                instanced_data_buffers[frame_index].get_buffer()
            };
            vkCmdBindVertexBuffers(command_buffer_handle, 0, static_cast<std::uint32_t>(buffer_handles.size()), buffer_handles.data(), offsets.data());
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(entry.mesh->get_number_of_vertices()), instance_count, 0, 0);
        }

        // Render debug bounding boxes (we do this after instanced data and switch pipelines again, because BBs are transparent so all opaque data needs to be rendered before)
        if (context.show_debug_bbs && !bounding_boxes_to_render.empty()) {
            vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
            vkCmdBindDescriptorSets(
                command_buffer.get_command_buffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->get_pipeline_layout(),
                0, 1,
                &descriptor_sets[frame_index],
                0, nullptr);
            for (const auto& entry: bounding_boxes_to_render) {
                PushConstants constants{ glm::mat4(1.0f), 1 };
                vkCmdPushConstants(
                    command_buffer_handle,
                    pipeline->get_pipeline_layout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0, sizeof(PushConstants),
                    &constants);

                static const VkDeviceSize offset = 0;
                static constexpr std::uint32_t vertices_per_bounding_box = 36;
                const auto buffer_handle = entry.get_buffer();
                vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &buffer_handle, &offset);
                vkCmdDraw(command_buffer_handle, vertices_per_bounding_box, 1, 0, 0);
            }
        }

        // Render particles
        if (!particles_to_render.empty()) {
            vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, particle_pipeline->get_pipeline());
            vkCmdBindDescriptorSets(
                command_buffer.get_command_buffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                particle_pipeline->get_pipeline_layout(),
                0, 1,
                &particle_descriptor_sets[frame_index],
                0, nullptr);
            
            // Upload uniform data
            ParticleMatrices particle_matrices;
            particle_matrices.projection_matrix = projection_matrix;
            particle_matrices.view_matrix = view_matrix;
            particle_matrices.inverse_view_matrix = glm::inverse(view_matrix);
            particle_matrices.ambient_light = ambient_light;
            particle_uniform_buffers[frame_index].upload(&particle_matrices, sizeof(ParticleMatrices));

            for (const auto& particle_system : particles_to_render) {
                std::array<VkDeviceSize, 2> offsets{ 0, 0 };
                std::array<VkBuffer, 2> buffer_handles {
                    particle_system.mesh->get_buffer().get_buffer(),
                    particle_data_buffers[frame_index].get_buffer()
                };
                const auto& positions = particle_system.positions;
                const auto instance_count = static_cast<std::uint32_t>(positions.size());
                const auto positions_num_bytes = instance_count * sizeof(glm::vec3);
                // TODO: This will NOT work when we are trying to render more than one particle system at a time
                particle_data_buffers[frame_index].upload(positions.data(), positions_num_bytes);
                vkCmdBindVertexBuffers(command_buffer_handle, 0, static_cast<std::uint32_t>(buffer_handles.size()), buffer_handles.data(), offsets.data());
                vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(particle_system.mesh->get_number_of_vertices()), instance_count, 0, 0);
            }
        }

        // Render imgui data
        ImGui::Render();
        const auto draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer.get_command_buffer());

        render_pass->end(command_buffer);
        command_buffer.end();
        command_buffer.submit(
            logical_device->get_graphics_queue(),
            image_available_semaphores[frame_index],
            render_finished_semaphores[frame_index],
            in_flight_fences[frame_index]);

        VkSemaphore render_finished_semaphore_handle = render_finished_semaphores[frame_index].get_semaphore();
        VkSwapchainKHR swap_chain_handle = swap_chain->get_swap_chain();
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_semaphore_handle;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain_handle;
        present_info.pImageIndices = &image_index;
        // TODO: This seems to return a non-successful result on Linux, but everything still works.
        // Check what is wrong and put back the result validation.
        vkQueuePresentKHR(logical_device->get_present_queue(), &present_info);

        frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    bool Renderer::is_in_view(const BoundingBox3D& bounding_box) const {
        // We expect the bounding box to already have the model matrix applied
        static constexpr std::array<glm::vec4, 8> ndc_points {
            glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),
            glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(1.0f, 1.0f, 0.0f, 1.f),
            glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
            glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
            glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        };
        // TODO: Change this to OBB frustum checking (https://bruop.github.io/improved_frustum_culling/) to be more accurate.
        const auto inverse_clip_matrix = glm::inverse(projection_matrix * camera.to_view_matrix());
        BoundingBox3D frustum_bb;
        for (const auto& point : ndc_points) {
            const auto result = inverse_clip_matrix * point;
            frustum_bb.update(glm::vec3(result / result.w));
        }

        return frustum_bb.collides(bounding_box);
    }

    const glm::mat4& Renderer::get_projection_matrix() const {
        return projection_matrix;
    }

    glm::mat4 Renderer::get_view_matrix() const {
        return camera.to_view_matrix();
    }

    Frustum Renderer::get_frustum_in_view_space() const {
        return Frustum(projection_matrix);
    }

    Frustum Renderer::get_frustum_in_world_space() const {
        return Frustum(projection_matrix * camera.to_view_matrix());
    }

    void Renderer::destroy_imgui() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Renderer::init_imgui(const Window& window, VkSampleCountFlagBits sample_count) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window.get_handle(), true);

        const auto queue_family_indices = physical_device->get_queue_family_indices();
        const auto& swap_chain_support = logical_device->get_swap_chain_support();

        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance = instance->get_instance();
        init_info.PhysicalDevice = physical_device->get_physical_device();
        init_info.Device = logical_device->get_device();
        init_info.QueueFamily = queue_family_indices.graphics_family.value();
        init_info.Queue = logical_device->get_graphics_queue();
        init_info.DescriptorPool = descriptor_pool->get_descriptor_pool();
        init_info.MinImageCount = swap_chain_support.surface_capabilities.minImageCount;
        init_info.ImageCount = swap_chain_support.surface_capabilities.minImageCount + 1;
        init_info.MSAASamples = sample_count;
        ImGui_ImplVulkan_Init(&init_info, render_pass->get_render_pass());
    }

}