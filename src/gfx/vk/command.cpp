#include "gfx/vk/command.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    CommandBuffer::CommandBuffer(const VkCommandBuffer& command_buffer) :
        command_buffer(command_buffer) {}

    VkCommandBuffer CommandBuffer::get_command_buffer() const {
        return command_buffer;
    }

    void CommandBuffer::reset() const {
        vkResetCommandBuffer(command_buffer, 0);
    }

    void CommandBuffer::begin() const {
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
            throw std::runtime_error("Failed to start recording to Vulkan command buffer.");
        }
    }

    void CommandBuffer::end() const {
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to stop recording to Vulkan command buffer.");
        }
    }

    void CommandBuffer::submit(
        VkQueue queue,
        const Semaphore& wait_semaphore,
        const Semaphore& signal_semaphore,
        const Fence& in_flight_fence) const {
        VkSemaphore wait_semaphore_handle = wait_semaphore.get_semaphore();
        VkSemaphore signal_semaphore_handle = signal_semaphore.get_semaphore();
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &wait_semaphore_handle;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signal_semaphore_handle;
        if (vkQueueSubmit(queue, 1, &submit_info, in_flight_fence.get_fence()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit Vulkan command buffer.");
        }
    }

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

    CommandBuffer CommandPool::allocate_buffer() const {
        VkCommandBufferAllocateInfo buffer_allocate_info{};
        buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_allocate_info.commandPool = command_pool;
        buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        buffer_allocate_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(device->get_device(), &buffer_allocate_info, &command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan command buffer.");
        }
        return CommandBuffer(command_buffer);
    }

}