#pragma once

#include <glad/vulkan.h>

#include <cstdint>

namespace inf::gfx::vk {

    // We need to forward declare here to avoid cyclic dependencies
    // Dependency chain: Device -> SwapChain -> ImageView -> Device
    struct LogicalDevice;
    struct PhysicalDevice;

    struct Image {

        static Image create(
            const LogicalDevice* logical_device,
            const PhysicalDevice& physical_device,
            std::uint32_t width,
            std::uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkSampleCountFlagBits samples);

        Image(const LogicalDevice* device, VkImage image, VkDeviceMemory image_memory);
        ~Image();
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&&);
        Image& operator=(Image&&);

        VkImage get_image() const;

    private:

        const LogicalDevice* device;
        VkImage image;
        VkDeviceMemory image_memory;

    };

    struct ImageView {

        static ImageView create(
            const LogicalDevice* device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspect_mask);

        ImageView(const LogicalDevice* device, VkImageView image_view);
        ~ImageView();
        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;
        ImageView(ImageView&&);
        ImageView& operator=(ImageView&&);

        VkImageView get_image_view() const;

    private:

        const LogicalDevice* device;
        VkImageView image_view;

    };

}