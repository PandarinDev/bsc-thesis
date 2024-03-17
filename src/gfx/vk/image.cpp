#include "gfx/vk/image.h"
#include "gfx/vk/device.h"

#include <vma.h>

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    Image Image::create(
        const MemoryAllocator* allocator,
        std::uint32_t width,
        std::uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkSampleCountFlagBits samples) {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = width;
        image_create_info.extent.height = height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.format = format;
        image_create_info.tiling = tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = usage;
        image_create_info.samples = samples;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocate_info{};
        allocate_info.usage = VMA_MEMORY_USAGE_AUTO;

        VkImage image;
        VmaAllocation allocation;        
        if (vmaCreateImage(allocator->get_allocator(), &image_create_info, &allocate_info, &image, &allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan image.");
        }
        return Image(allocator, image, allocation);
    }

    Image::Image(const MemoryAllocator* allocator, VkImage image, VmaAllocation allocation) :
        allocator(allocator),
        image(image),
        allocation(allocation) {}

    Image::~Image() {
        if (allocator) {
            vmaDestroyImage(allocator->get_allocator(), image, allocation);
        }
    }

    Image::Image(Image&& other) :
        allocator(std::exchange(other.allocator, nullptr)),
        image(std::exchange(other.image, nullptr)),
        allocation(std::exchange(other.allocation, nullptr)) {}

    Image& Image::operator=(Image&& other) {
        allocator = std::exchange(other.allocator, nullptr);
        image = std::exchange(other.image, nullptr);
        allocation = std::exchange(other.allocation, nullptr);

        return *this;
    }

    VkImage Image::get_image() const {
        return image;
    }

    ImageView ImageView::create(
        const LogicalDevice* device,
        VkImage image,
        VkFormat format,
        VkImageAspectFlags aspect_mask) {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = aspect_mask;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        
        VkImageView image_view;
        if (vkCreateImageView(device->get_device(), &image_view_create_info, nullptr, &image_view) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan image view.");
        }
        return ImageView(device, image_view);
    }

    ImageView::ImageView(const LogicalDevice* device, VkImageView image_view) :
        device(device),
        image_view(image_view) {}

    ImageView::~ImageView() {
        if (device) {
            vkDestroyImageView(device->get_device(), image_view, nullptr);
        }
    }

    ImageView::ImageView(ImageView&& other) :
        device(std::exchange(other.device, nullptr)),
        image_view(std::exchange(other.image_view, VK_NULL_HANDLE)) {}

    ImageView& ImageView::operator=(ImageView&& other) {
        device = std::exchange(other.device, nullptr);
        image_view = std::exchange(other.image_view, VK_NULL_HANDLE);

        return *this;
    }

    VkImageView ImageView::get_image_view() const {
        return image_view;
    }

}