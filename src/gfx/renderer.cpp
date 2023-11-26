#include "gfx/renderer.h"

#include <glad/vulkan.h>

#include <stdexcept>

namespace inf::gfx {

    Renderer::Renderer() {
        if (!gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE)) {
            throw std::runtime_error("Failed to load Vulkan function pointers.");
        }
        instance = std::make_unique<vk::Instance>(vk::Instance::create_instance("infinitown"));
        device = vk::Device::choose_optimal_device(*instance);
    }

}