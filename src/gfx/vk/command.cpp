#include "gfx/vk/command.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    CommandPool CommandPool::create_command_pool(
        const LogicalDevice* device,
        const QueueFamilyIndices& queue_families) {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = queue_families.graphics_family.value();

        VkCommandPool command_pool;
        if (vkCreateCommandPool(device->get_device(), &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan command pool.");
        }
        return CommandPool(device, command_pool);
    }

    CommandPool::CommandPool(const LogicalDevice* device, const VkCommandPool& command_pool) :
        device(device),
        command_pool(command_pool) {}

    CommandPool::~CommandPool() {
        if (device) {
            vkDestroyCommandPool(device->get_device(), command_pool, nullptr);
        }
    }

    CommandPool::CommandPool(CommandPool&& other) :
        device(std::exchange(other.device, nullptr)),
        command_pool(std::exchange(other.command_pool, VK_NULL_HANDLE)) {}

    CommandPool& CommandPool::operator=(CommandPool&& other) {
        device = std::exchange(other.device, nullptr);
        command_pool = std::exchange(other.command_pool, VK_NULL_HANDLE);

        return *this;
    }

    VkCommandPool CommandPool::get_command_pool() const {
        return command_pool;
    }

    VkCommandBuffer CommandPool::allocate_buffer() const {
        VkCommandBufferAllocateInfo buffer_allocate_info{};
        buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_allocate_info.commandPool = command_pool;
        buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        buffer_allocate_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(device->get_device(), &buffer_allocate_info, &command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan command buffer.");
        }
        return command_buffer;
    }

}