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
        for (std::size_t i = 0; i < num_images; ++i) {
            image_views.emplace_back(ImageView::create(device, images[i], image_format, VK_IMAGE_ASPECT_COLOR_BIT));
        }
    }

    SwapChain::~SwapChain() {
        if (device) {
            vkDestroySwapchainKHR(device->get_device(), swap_chain, nullptr);
        }
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

    const std::vector<ImageView>& SwapChain::get_image_views() const {
        return image_views;
    }

}