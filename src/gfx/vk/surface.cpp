#include "gfx/vk/surface.h"

#include <utility>

namespace inf::gfx::vk {

    Surface::Surface(const Instance* instance, const VkSurfaceKHR& surface) :
        instance(instance),
        surface(surface) {}

    Surface::~Surface() {
        // All swapchain objects created for this surface must be destroyed before this call
        vkDestroySurfaceKHR(instance->get_instance(), surface, nullptr);
    }

    Surface::Surface(Surface&& other) :
        instance(other.instance),
        surface(std::exchange(other.surface, VK_NULL_HANDLE)) {}

    Surface& Surface::operator=(Surface&& other) {
        instance = other.instance;
        surface = std::exchange(other.surface, VK_NULL_HANDLE);

        return *this;
    }

    const VkSurfaceKHR& Surface::get_surface() const {
        return surface;
    }

}