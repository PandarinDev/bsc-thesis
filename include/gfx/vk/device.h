#pragma once

#include "gfx/vk/instance.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct PhysicalDevice {

        PhysicalDevice(const VkPhysicalDevice& device);

        bool is_dedicated_gpu() const;

    private:

        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;

    };

    struct Device {

        Device() = delete;

        static VkPhysicalDevice choose_optimal_device(const Instance& instance);

    };

}