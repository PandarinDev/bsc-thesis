#pragma once

#include "gfx/vk/device.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct CommandPool {

        static CommandPool create_command_pool(
            const LogicalDevice* device,
            const QueueFamilyIndices& queue_families);

        CommandPool(const LogicalDevice* device, const VkCommandPool& command_pool);
        ~CommandPool();
        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;
        CommandPool(CommandPool&&);
        CommandPool& operator=(CommandPool&&);

        VkCommandPool get_command_pool() const;
        VkCommandBuffer allocate_buffer() const;

    private:

        const LogicalDevice* device;
        VkCommandPool command_pool;

    };

}