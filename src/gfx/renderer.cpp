#include "gfx/renderer.h"
#include "utils/file_utils.h"

#include <glad/vulkan.h>

#include <stdexcept>

namespace inf::gfx {

    Renderer::Renderer(const Window& window) {
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

        render_pass = std::make_unique<vk::RenderPass>(vk::RenderPass::create_render_pass(logical_device.get(), swap_chain->get_format()));
        pipeline = std::make_unique<vk::Pipeline>(vk::Pipeline::create_pipeline(logical_device.get(), *render_pass, swap_chain->get_extent(), shaders));
    }

    const vk::Instance& Renderer::get_vulkan_instance() const {
        return *instance;
    }

}