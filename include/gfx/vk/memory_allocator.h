#pragma once

#include "vma.h"

#include "gfx/vk/device.h"

namespace inf::gfx::vk {

    struct MemoryAllocator {

        static MemoryAllocator create(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device);

        MemoryAllocator(VmaAllocator allocator);
        ~MemoryAllocator();
        MemoryAllocator(const MemoryAllocator&) = delete;
        MemoryAllocator& operator=(const MemoryAllocator&) = delete;
        MemoryAllocator(MemoryAllocator&&);
        MemoryAllocator& operator=(MemoryAllocator&&);

        VmaAllocator get_allocator() const;

    private:

        VmaAllocator allocator;

    };

}