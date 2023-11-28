#pragma once

#include "gfx/vk/device.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct Semaphore {

        static Semaphore create(const LogicalDevice* device);

        Semaphore(const LogicalDevice* device, const VkSemaphore& semaphore);
        ~Semaphore();
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore(Semaphore&&);
        Semaphore& operator=(Semaphore&&);

        VkSemaphore get_semaphore() const;

    private:

        const LogicalDevice* device;
        VkSemaphore semaphore;

    };

    struct Fence {

        static Fence create(const LogicalDevice* device, bool create_signaled);

        Fence(const LogicalDevice* device, const VkFence& fence);
        ~Fence();
        Fence(const Fence&) = delete;
        Fence& operator=(const Fence&) = delete;
        Fence(Fence&&);
        Fence& operator=(Fence&&);

        VkFence get_fence() const;

        void wait_for() const;
        void reset() const;
        void wait_for_and_reset() const;

    private:

        const LogicalDevice* device;
        VkFence fence;

    };

}