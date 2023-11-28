#include "gfx/vk/semaphore.h"

#include <utility>
#include <stdexcept>
#include <limits>

namespace inf::gfx::vk {

    Semaphore Semaphore::create(const LogicalDevice* device) {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkSemaphore semaphore;
        if (vkCreateSemaphore(device->get_device(), &semaphore_create_info, nullptr, &semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan semaphore.");
        }
        return Semaphore(device, semaphore);
    }

    Semaphore::Semaphore(const LogicalDevice* device, const VkSemaphore& semaphore) :
        device(device),
        semaphore(semaphore) {}

    Semaphore::~Semaphore() {
        if (device) {
            vkDestroySemaphore(device->get_device(), semaphore, nullptr);
        }
    }

    Semaphore::Semaphore(Semaphore&& other) :
        device(std::exchange(other.device, nullptr)),
        semaphore(std::exchange(other.semaphore, VK_NULL_HANDLE)) {}

    Semaphore& Semaphore::operator=(Semaphore&& other) {
        device = std::exchange(other.device, nullptr);
        semaphore = std::exchange(other.semaphore, VK_NULL_HANDLE);

        return *this;
    }

    Fence Fence::create(const LogicalDevice* device, bool create_signaled) {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = create_signaled
            ? VK_FENCE_CREATE_SIGNALED_BIT
            : 0;

        VkFence fence;
        if (vkCreateFence(device->get_device(), &fence_create_info, nullptr, &fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan fence.");
        }
        return Fence(device, fence);
    }

    Fence::Fence(const LogicalDevice* device, const VkFence& fence) :
        device(device),
        fence(fence) {}

    Fence::~Fence() {
        if (device) {
            vkDestroyFence(device->get_device(), fence, nullptr);
        }
    }

    Fence::Fence(Fence&& other) :
        device(std::exchange(other.device, nullptr)),
        fence(std::exchange(other.fence, VK_NULL_HANDLE)) {}

    Fence& Fence::operator=(Fence&& other) {
        device = std::exchange(other.device, nullptr);
        fence = std::exchange(other.fence, VK_NULL_HANDLE);

        return *this;
    }

    void Fence::wait_for() const {
        vkWaitForFences(device->get_device(), 1, &fence, VK_FALSE, std::numeric_limits<std::uint64_t>::max());
    }

    void Fence::reset() const {
        vkResetFences(device->get_device(), 1, &fence);
    }

    void Fence::wait_for_and_reset() const {
        wait_for();
        reset();
    }

}