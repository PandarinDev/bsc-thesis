#include "gfx/renderer.h"

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
        logical_device = std::make_unique<vk::LogicalDevice>(physical_device->create_logical_device());
    }

    const vk::Instance& Renderer::get_vulkan_instance() const {
        return *instance;
    }

}