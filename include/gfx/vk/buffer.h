#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/memory_allocator.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    enum class BufferType {
        UNIFORM_BUFFER = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VERTEX_BUFFER = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    struct MappedBuffer {

        static MappedBuffer create(
            const LogicalDevice* logical_device,
            const MemoryAllocator* allocator,
            BufferType type,
            std::uint64_t size);

        MappedBuffer(
            const LogicalDevice* device,
            const MemoryAllocator* allocator,
            const VkBuffer& buffer,
            const VmaAllocation& allocation);
        ~MappedBuffer();
        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer& operator=(const MappedBuffer&) = delete;
        MappedBuffer(MappedBuffer&&);
        MappedBuffer& operator=(MappedBuffer&&);

        VkBuffer get_buffer() const;

        void upload(const void* data, std::size_t size) const;

    private:

        const LogicalDevice* device;
        const MemoryAllocator* allocator;
        VkBuffer buffer;
        VmaAllocation allocation;

    };

}