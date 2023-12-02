#include "gfx/vk/image.h"
#include "gfx/vk/device.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    Image Image::create(
        const LogicalDevice* logical_device,
        const PhysicalDevice& physical_device,
        std::uint32_t width,
        std::uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage) {
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
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkImage image;
        if (vkCreateImage(logical_device->get_device(), &image_create_info, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan image.");
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical_device->get_device(), image, &memory_requirements);

        const auto memory_type_index = physical_device.get_memory_type_index(memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = memory_type_index.value();
        VkDeviceMemory image_memory;
        if (vkAllocateMemory(logical_device->get_device(), &allocate_info, nullptr, &image_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate memory for Vulkan image.");
        }
        vkBindImageMemory(logical_device->get_device(), image, image_memory, 0);

        return Image(logical_device, image, image_memory);
    }

    Image::Image(const LogicalDevice* device, VkImage image, VkDeviceMemory image_memory) :
        device(device),
        image(image),
        image_memory(image_memory) {}

    Image::~Image() {
        if (device) {
            vkDestroyImage(device->get_device(), image, nullptr);
            vkFreeMemory(device->get_device(), image_memory, nullptr);
        }
    }

    Image::Image(Image&& other) :
        device(std::exchange(other.device, nullptr)),
        image(std::exchange(other.image, VK_NULL_HANDLE)),
        image_memory(std::exchange(other.image_memory, VK_NULL_HANDLE)) {}

    Image& Image::operator=(Image&& other) {
        device = std::exchange(other.device, nullptr);
        image = std::exchange(other.image, VK_NULL_HANDLE);
        image_memory = std::exchange(other.image_memory, VK_NULL_HANDLE);

        return *this;
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