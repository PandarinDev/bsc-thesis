#include "gfx/renderer.h"
#include "utils/file_utils.h"

#include <glad/vulkan.h>

#include <limits>
#include <stdexcept>

namespace inf::gfx {

    Renderer::Renderer(const Window& window, const Camera& camera) :
        camera(camera), image_index(0), frame_index(0) {
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
        swap_chain = std::make_unique<vk::SwapChain>(logical_device->create_swap_chain(*surface));

        // Load shaders
        {
            const auto vertex_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/default.vert.bin");
            shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, vertex_shader_bytes));
            
            const auto fragment_shader_bytes = utils::FileUtils::read_bytes("assets/shaders/default.frag.bin");
            shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, fragment_shader_bytes));

            const auto shadow_map_vs_bytes = utils::FileUtils::read_bytes("assets/shaders/shadow_map.vert.bin");
            shadow_map_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::VERTEX, shadow_map_vs_bytes));
            
            const auto shadow_map_fs_bytes = utils::FileUtils::read_bytes("assets/shaders/shadow_map.frag.bin");
            shadow_map_shaders.emplace_back(vk::Shader::create_from_bytes(logical_device.get(), vk::ShaderType::FRAGMENT, shadow_map_fs_bytes));
        }

        // Create descriptor pool and set layouts for shader uniform data
        descriptor_pool = std::make_unique<vk::DescriptorPool>(vk::DescriptorPool::create(logical_device.get(), 3));
        descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            }));
        shadow_map_descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(
            logical_device.get(), {
                VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
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
        pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(), *render_pass, swap_chain->get_extent(), *descriptor_set_layout, shaders, sample_count, std::nullopt));
        shadow_map_pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(
            logical_device.get(), *shadow_map_render_pass, swap_chain->get_extent(), *shadow_map_descriptor_set_layout,
            shadow_map_shaders, VK_SAMPLE_COUNT_1_BIT, gfx::vk::PipelineDepthBias{ 1.25f, 1.75f }));

        // Create a separate color image if necessary because of multisampling
        // If not necessary (sample count = 1), we use the swapchain image instead.
        if (sample_count > VK_SAMPLE_COUNT_1_BIT) {
            color_image = std::make_unique<vk::Image>(vk::Image::create(
                logical_device.get(),
                *physical_device,
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
            logical_device.get(), *physical_device, swap_chain_extent, sample_count, false));
        const auto& depth_image_view = depth_buffer->get_image_view();

        // Create a separate depth buffer for the shadow map that will be sampled during the second render pass
        shadow_map_depth_buffer = std::make_unique<vk::DepthBuffer>(vk::DepthBuffer::create(
            logical_device.get(), *physical_device, swap_chain_extent, VK_SAMPLE_COUNT_1_BIT, true));

        // Create framebuffers for swapchain images
        for (const auto& image_view : swap_chain->get_image_views()) {
            framebuffers.emplace_back(vk::Framebuffer::create_from_image_view(
                logical_device.get(), *render_pass, image_view, depth_image_view, color_image_view.get(), swap_chain_extent));
        }

        // Create framebuffers for the shadow map
        shadow_map_framebuffer = std::make_unique<vk::Framebuffer>(vk::Framebuffer::create_for_shadow_map(
            logical_device.get(), *shadow_map_render_pass, shadow_map_depth_buffer->get_image_view(), swap_chain_extent));

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
            uniform_buffers.emplace_back(vk::MappedBuffer::create(*physical_device, logical_device.get(), vk::BufferType::UNIFORM_BUFFER, sizeof(Matrices)));
        }
        shadow_map_uniform_buffer = std::make_unique<vk::MappedBuffer>(vk::MappedBuffer::create(
            *physical_device, logical_device.get(), vk::BufferType::UNIFORM_BUFFER, sizeof(Matrices)));

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
        
        VkDescriptorBufferInfo shadow_map_buffer_info;
        shadow_map_buffer_info.buffer = shadow_map_uniform_buffer->get_buffer();
        shadow_map_buffer_info.offset = 0;
        shadow_map_buffer_info.range = VK_WHOLE_SIZE;
        std::vector<std::vector<VkWriteDescriptorSet>> shadow_map_write_descriptor_set = {
            { gfx::vk::WriteDescriptorSet::create_for_buffer(shadow_map_buffer_info, 0) }  
        };
        shadow_map_descriptor_set = descriptor_pool->allocate_sets(
            *shadow_map_descriptor_set_layout, shadow_map_write_descriptor_set, 1)[0];

        projection_matrix = glm::perspective(
            FOVY,
            static_cast<float>(swap_chain_extent.width) / swap_chain_extent.height,
            NEAR_PLANE,
            FAR_PLANE);
        // Flip the Y axis to account for the degenerate coordinate system of Vulkan
        projection_matrix[1][1] *= -1.0f;

        // Because we are using directional shadows the projection needs to be orthographic
        // TODO: Left, right, top, bottom needs to be calculated dynamically to fit content.
        float aspect_ratio = static_cast<float>(swap_chain_extent.width) / swap_chain_extent.height;
        shadow_map_projection_matrix = glm::ortho(-20.0f, 20.0f, 20.0f / aspect_ratio, -20.0f / aspect_ratio, 0.01f, 100.0f);
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

    void Renderer::begin_frame() {
        meshes_to_draw.clear();
    }

    void Renderer::render(const Mesh& mesh) {
        meshes_to_draw.emplace_back(MeshToRender{ &mesh, mesh.get_model_matrix() });
    }

    void Renderer::end_frame(const BoundingBox3D& bounding_box) {
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

        const auto& extent = swap_chain->get_extent();
        const auto& command_buffer = command_buffers[frame_index];
        command_buffer.reset();
        command_buffer.begin();

        // In the first render pass we render into a shadow map which will be sampled in the second render pass
        std::vector<VkClearValue> shadow_map_clear_values(1);
        shadow_map_clear_values[0].depthStencil = { 1.0f, 0 };
        shadow_map_render_pass->begin(*shadow_map_framebuffer, extent, command_buffer, shadow_map_clear_values);
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_map_pipeline->get_pipeline());
        // Since the viewport and the scissor is dynamic we need to supply it each frame
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

        // Bind descriptor sets
        vkCmdBindDescriptorSets(
            command_buffer.get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            shadow_map_pipeline->get_pipeline_layout(),
            0, 1,
            &shadow_map_descriptor_set,
            0, nullptr);

        // Upload uniform buffer data
        Matrices shadow_map_matrices;
        shadow_map_matrices.projection_matrix = shadow_map_projection_matrix;
        glm::mat4 sun_view_matrix = glm::lookAt(
            glm::vec3(bounding_box.max.x + 2.5f, bounding_box.max.y + 12.0f, bounding_box.max.z),
            bounding_box.center(),
            glm::vec3(0.0f, 1.0f, 0.0f));
        shadow_map_matrices.view_matrix = sun_view_matrix;
        shadow_map_matrices.light_space_matrix = glm::mat4(1.0f);
        shadow_map_uniform_buffer->upload(shadow_map_matrices);

        const auto command_buffer_handle = command_buffer.get_command_buffer();
        for (const auto& entry : meshes_to_draw) {
            const auto mesh = entry.mesh;
            // Push model matrix
            vkCmdPushConstants(
                command_buffer_handle,
                shadow_map_pipeline->get_pipeline_layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(glm::mat4),
                &entry.model_matrix);

            static const VkDeviceSize offset = 0;
            const auto vertex_buffer = mesh->get_buffer().get_buffer();
            vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &vertex_buffer, &offset);
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(mesh->get_number_of_vertices()), 1, 0, 0);
        }

        shadow_map_render_pass->end(command_buffer);

        // In the second render pass we render color data
        std::vector<VkClearValue> clear_values(2);
        clear_values[0].color = {{ 0.0f, 0.1f, 0.95f, 1.0f }};
        clear_values[1].depthStencil = { 1.0f, 0 };
        render_pass->begin(framebuffers[image_index], extent, command_buffer, clear_values);
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
        vkCmdSetViewport(command_buffer.get_command_buffer(), 0, 1, &viewport);
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
        matrices.view_matrix = camera.to_view_matrix();
        matrices.light_space_matrix = shadow_map_projection_matrix * sun_view_matrix;
        uniform_buffers[frame_index].upload(matrices);

        // Render the meshes
        for (const auto entry : meshes_to_draw) {
            const auto mesh = entry.mesh;
            // Push model matrix
            vkCmdPushConstants(
                command_buffer_handle,
                pipeline->get_pipeline_layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(glm::mat4),
                &entry.model_matrix);

            static const VkDeviceSize offset = 0;
            const auto vertex_buffer = mesh->get_buffer().get_buffer();
            vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &vertex_buffer, &offset);
            vkCmdDraw(command_buffer_handle, static_cast<std::uint32_t>(mesh->get_number_of_vertices()), 1, 0, 0);
        }

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
        if (vkQueuePresentKHR(logical_device->get_present_queue(), &present_info) != VK_SUCCESS) {
            throw std::runtime_error("Failed to present Vulkan image.");
        }

        frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}