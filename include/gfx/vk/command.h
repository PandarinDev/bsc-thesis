#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/semaphore.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct CommandBuffer {

        CommandBuffer(const VkCommandBuffer& command_buffer);

        VkCommandBuffer get_command_buffer() const;

        void reset() const;
        void begin() const;
        void end() const;
        void submit(
            VkQueue queue,
            const Semaphore& wait_semaphore,
            const Semaphore& signal_semaphore,
            const Fence& in_flight_fence) const;

    private:

        VkCommandBuffer command_buffer;

    };

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
        CommandBuffer allocate_buffer() const;

    private:

        const LogicalDevice* device;
        VkCommandPool command_pool;

    };

}