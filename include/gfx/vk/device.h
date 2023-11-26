#pragma once

#include "gfx/vk/instance.h"
#include "gfx/vk/surface.h"

#include <glad/vulkan.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace inf::gfx::vk {

    struct QueueFamilyIndices {

        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> presentation_family;

        bool is_complete() const;
        std::vector<VkDeviceQueueCreateInfo> to_queue_create_info() const;

    };

    struct LogicalDevice {

        explicit LogicalDevice(const VkDevice& device, const QueueFamilyIndices& queue_family_indices);
        ~LogicalDevice();
        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice& operator=(const LogicalDevice&) = delete;
        LogicalDevice(LogicalDevice&&);
        LogicalDevice& operator=(LogicalDevice&&);

        VkQueue get_graphics_queue() const;

    private:

        [[maybe_unused]] VkDevice device;
        QueueFamilyIndices queue_family_indices;

    };

    struct PhysicalDevice {

        explicit PhysicalDevice(const VkPhysicalDevice& device, const Surface& surface);

        bool is_dedicated_gpu() const;
        bool is_suitable() const;
        const QueueFamilyIndices& get_queue_family_indices() const;
        LogicalDevice create_logical_device() const;

    private:

        [[maybe_unused]] VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        QueueFamilyIndices queue_family_indices;

    };

    struct Device {

        Device() = delete;

        static PhysicalDevice choose_optimal_device(const Instance& instance, const Surface& surface);

    };

}