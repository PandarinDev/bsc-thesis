#pragma once

#include <glad/vulkan.h>

#include <vector>

namespace inf::gfx::vk {

    // Forward declaration to avoid circular dependencies
    struct LogicalDevice;

    struct SwapChain {

        // The logical device must remain valid through the lifetime of the swap chain
        SwapChain(
            const VkSwapchainKHR& swap_chain,
            const LogicalDevice* device,
            VkFormat image_format,
            const VkExtent2D& image_extent);
        ~SwapChain();
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain(SwapChain&&);
        SwapChain& operator=(SwapChain&&);
    
        const VkSwapchainKHR& get_swap_chain() const;
        VkFormat get_format() const;
        const VkExtent2D& get_extent() const;

    private:

        VkSwapchainKHR swap_chain;
        const LogicalDevice* device;
        VkFormat image_format;
        VkExtent2D image_extent;
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;

    };

}