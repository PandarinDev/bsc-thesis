#pragma once

#include "gfx/vk/instance.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct Surface {

        // The received instance object must remain valid through the lifetime of the surface
        Surface(const Instance* instance, const VkSurfaceKHR& surface);
        ~Surface();
        Surface(const Surface&) = delete;
        Surface& operator=(const Surface&) = delete;
        Surface(Surface&&);
        Surface& operator=(Surface&& other);

        const VkSurfaceKHR& get_surface() const;

    private:

        const Instance* instance;
        VkSurfaceKHR surface;

    };

}