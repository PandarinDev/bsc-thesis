#include "gfx/vk/swap_chain.h"
#include "gfx/vk/device.h"

#include <utility>

namespace inf::gfx::vk {

    SwapChain::SwapChain(
        const VkSwapchainKHR& swap_chain,
        const LogicalDevice* device,
        VkFormat image_format,
        const VkExtent2D& image_extent) :
        swap_chain(swap_chain),
        device(device),
        image_format(image_format),
        image_extent(image_extent) {
        // Query images
        std::uint32_t num_images = 0;
        vkGetSwapchainImagesKHR(device->get_device(), swap_chain, &num_images, nullptr);
        images.resize(num_images);
        vkGetSwapchainImagesKHR(device->get_device(), swap_chain, &num_images, images.data());

        // Create image views
        image_views.resize(num_images);
        for (std::size_t i = 0; i < image_views.size(); ++i) {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            
            if (vkCreateImageView(device->get_device(), &image_view_create_info, nullptr, &image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Vulkan image view.");
            }
        }
    }

    SwapChain::~SwapChain() {
        // If the device is nullptr this swap chain was already moved, nothing to clean up
        if (!device) {
            return;
        }
        for (const auto& image_view : image_views) {
            vkDestroyImageView(device->get_device(), image_view, nullptr);
        }
        vkDestroySwapchainKHR(device->get_device(), swap_chain, nullptr);
    }

    SwapChain::SwapChain(SwapChain&& other) :
        swap_chain(std::exchange(other.swap_chain, VK_NULL_HANDLE)),
        device(std::exchange(other.device, nullptr)),
        image_format(other.image_format),
        image_extent(std::move(other.image_extent)),
        images(std::move(other.images)),
        image_views(std::move(other.image_views)) {}

    SwapChain& SwapChain::operator=(SwapChain&& other) {
        swap_chain = std::exchange(other.swap_chain, VK_NULL_HANDLE);
        device = std::exchange(other.device, nullptr);
        image_format = other.image_format;
        image_extent = std::move(other.image_extent);
        images = std::move(other.images);
        image_views = std::move(other.image_views);

        return *this;
    }

    const VkSwapchainKHR& SwapChain::get_swap_chain() const {
        return swap_chain;
    }

    VkFormat SwapChain::get_format() const {
        return image_format;
    }

    const VkExtent2D& SwapChain::get_extent() const {
        return image_extent;
    }

}