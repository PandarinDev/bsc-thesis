#include "renderer.h"

#include <glad/vulkan.h>

#include <stdexcept>

namespace inf {

    Renderer::Renderer() {
        if (gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE) < 0) {
            throw std::runtime_error("Failed to load Vulkan function pointers.");
        }
    }

}