#include "gfx/vk/memory_allocator.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    MemoryAllocator MemoryAllocator::create(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device) {
        VmaAllocatorCreateInfo allocator_create_info{};
        allocator_create_info.instance = instance;
        allocator_create_info.physicalDevice = physical_device;
        allocator_create_info.device = logical_device;
        
        VmaVulkanFunctions vk_functions{};
        vk_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vk_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vk_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vk_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vk_functions.vkAllocateMemory = vkAllocateMemory;
        vk_functions.vkFreeMemory = vkFreeMemory;
        vk_functions.vkMapMemory = vkMapMemory;
        vk_functions.vkUnmapMemory = vkUnmapMemory;
        vk_functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vk_functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vk_functions.vkBindBufferMemory = vkBindBufferMemory;
        vk_functions.vkBindImageMemory = vkBindImageMemory;
        vk_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vk_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vk_functions.vkCreateBuffer = vkCreateBuffer;
        vk_functions.vkDestroyBuffer = vkDestroyBuffer;
        vk_functions.vkCreateImage = vkCreateImage;
        vk_functions.vkDestroyImage = vkDestroyImage;
        vk_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        // TODO: When upgrading VMA to Vulkan 1.1+ add the remaining function ptrs here
        allocator_create_info.pVulkanFunctions = &vk_functions;

        VmaAllocator allocator;
        if (vmaCreateAllocator(&allocator_create_info, &allocator) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan memory allocator.");
        }
        return MemoryAllocator(allocator);
    }
 
    MemoryAllocator::MemoryAllocator(VmaAllocator allocator) : allocator(allocator) {}
    
    MemoryAllocator::~MemoryAllocator() {
        vmaDestroyAllocator(allocator);
    }
    
    MemoryAllocator::MemoryAllocator(MemoryAllocator&& other) : allocator(std::exchange(other.allocator, nullptr)) {}
    
    MemoryAllocator& MemoryAllocator::operator=(MemoryAllocator&& other) {
        this->allocator = std::exchange(other.allocator, nullptr);
        return *this;
    }

    VmaAllocator MemoryAllocator::get_allocator() const {
        return allocator;
    }

}