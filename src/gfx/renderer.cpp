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
        }

        // Create descriptor pool and set layouts for shader uniform data
        descriptor_pool = std::make_unique<vk::DescriptorPool>(vk::DescriptorPool::create(logical_device.get(), MAX_FRAMES_IN_FLIGHT));
        descriptor_set_layout = std::make_unique<vk::DescriptorSetLayout>(vk::DescriptorSetLayout::create(logical_device.get(), 0, vk::ShaderType::VERTEX));

        // Create render pass and graphics pipeline
        render_pass = std::make_unique<vk::RenderPass>(vk::RenderPass::create_render_pass(logical_device.get(), swap_chain->get_format()));
        pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(logical_device.get(), *render_pass, swap_chain->get_extent(), *descriptor_set_layout, shaders));

        // Create framebuffers for swapchain images
        const auto& swap_chain_extent = swap_chain->get_extent();
        for (const auto& image_view : swap_chain->get_image_views()) {
            framebuffers.emplace_back(vk::Framebuffer::create_from_image_view(logical_device.get(), *render_pass, image_view, swap_chain_extent));
        }

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

        // Allocate descriptor sets for the uniform buffers
        std::vector<VkBuffer> uniform_buffer_handles(uniform_buffers.size());
        for (std::size_t i = 0; i < uniform_buffers.size(); ++i) {
            uniform_buffer_handles[i] = uniform_buffers[i].get_buffer();
        }
        descriptor_sets = descriptor_pool->allocate_sets_for_buffers(*descriptor_set_layout, uniform_buffer_handles);

        projection_matrix = glm::perspective(
            FOVY,
            static_cast<float>(swap_chain_extent.width) / swap_chain_extent.height,
            NEAR_PLANE,
            FAR_PLANE);
        // Flip the Y axis to account for the degenerate coordinate system of Vulkan
        projection_matrix[1][1] *= -1.0f;
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

    Frustum Renderer::build_frustum() const {
        return Frustum(projection_matrix * camera.to_view_matrix());
    }

    void Renderer::begin_frame() {
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

        render_pass->begin(framebuffers[image_index], extent, command_buffer);
        vkCmdBindPipeline(command_buffer.get_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
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
            pipeline->get_pipeline_layout(),
            0, 1,
            &descriptor_sets[frame_index],
            0, nullptr);
    }

    void Renderer::render(const Mesh& mesh) const {
        // Upload uniform buffer data
        Matrices matrices;
        matrices.projection_matrix = projection_matrix;
        matrices.view_matrix = camera.to_view_matrix();
        matrices.model_matrix = mesh.get_model_matrix();
        uniform_buffers[frame_index].upload(matrices);

        static const VkDeviceSize offset = 0;
        const auto command_buffer = command_buffers[frame_index].get_command_buffer();
        const auto vertex_buffer = mesh.get_buffer().get_buffer();
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);
        vkCmdDraw(command_buffer, mesh.get_number_of_vertices(), 1, 0, 0);
    }

    void Renderer::end_frame() {
        const auto& command_buffer = command_buffers[frame_index];
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