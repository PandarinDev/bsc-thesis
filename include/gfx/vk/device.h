#pragma once

#include "gfx/vk/instance.h"
#include "gfx/vk/surface.h"
#include "gfx/vk/swap_chain.h"

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

    struct SwapChainSupport {

        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;

    };

    struct LogicalDevice {

        explicit LogicalDevice(
            const VkDevice& device,
            const QueueFamilyIndices& queue_family_indices,
            const SwapChainSupport& swap_chain_support);
        ~LogicalDevice();
        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice& operator=(const LogicalDevice&) = delete;
        LogicalDevice(LogicalDevice&&);
        LogicalDevice& operator=(LogicalDevice&&);

        const VkDevice& get_device() const;
        VkQueue get_graphics_queue() const;
        VkQueue get_present_queue() const;

        SwapChain create_swap_chain(const Surface& surface) const;
        void wait_until_idle() const;

    private:

        VkDevice device;
        QueueFamilyIndices queue_family_indices;
        SwapChainSupport swap_chain_support;

        VkSurfaceFormatKHR choose_surface_format() const;
        VkPresentModeKHR choose_present_mode() const;
        VkExtent2D choose_extent() const;

    };

    struct PhysicalDevice {

        explicit PhysicalDevice(const VkPhysicalDevice& device, const Surface& surface);

        const VkPhysicalDevice& get_physical_device() const;
        bool is_dedicated_gpu() const;
        bool is_suitable(const Surface& surface) const;
        const QueueFamilyIndices& get_queue_family_indices() const;
        LogicalDevice create_logical_device(const Surface& surface) const;

        // This is only a valid operation if the physical device supports the surface format
        SwapChainSupport query_swap_chain_support(const Surface& surface) const;

    private:

        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        bool supports_required_extensions;
        QueueFamilyIndices queue_family_indices;

    };

    struct Device {

        Device() = delete;

        static PhysicalDevice choose_optimal_device(const Instance& instance, const Surface& surface);

    };

}