#include "gfx/vk/buffer.h"
#include "gfx/renderer.h"

#include "vma.h"

#include <utility>
#include <stdexcept>
#include <optional>
#include <cstring>
#include <string>

namespace inf::gfx::vk {

    MappedBuffer MappedBuffer::create(
        const LogicalDevice* logical_device,
        const MemoryAllocator* allocator,
        BufferType type,
        std::uint64_t size) {
        // Create the buffer
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size;
        buffer_create_info.usage = static_cast<VkBufferUsageFlags>(type);
        // buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocation_info{};
        allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
        allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer buffer;
        VmaAllocation allocation;
        if (const auto result = vmaCreateBuffer(
            allocator->get_allocator(), &buffer_create_info, &allocation_info, &buffer, &allocation, nullptr); result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan buffer: " + std::to_string(result));
        }

        return MappedBuffer(logical_device, allocator, buffer, allocation, size);
    }

    MappedBuffer::MappedBuffer(
        const LogicalDevice* device,
        const MemoryAllocator* allocator,
        const VkBuffer& buffer,
        const VmaAllocation& allocation,
        std::uint64_t size) :
        device(device),
        allocator(allocator),
        buffer(buffer),
        allocation(allocation),
        size(size) {}

    MappedBuffer::~MappedBuffer() {
        if (device) {
            // We need to wait for the currently in-flight frame to finish, otherwise the buffer might still be in use
            // TODO: This is really bad from a performance point of view. Instead we should be adding the handles to
            // be freed to a collection and free them all at the end of the frame.
            device->wait_until_idle();
            vmaDestroyBuffer(allocator->get_allocator(), buffer, allocation);
        }
    }

    MappedBuffer::MappedBuffer(MappedBuffer&& other) :
        device(std::exchange(other.device, nullptr)),
        allocator(std::exchange(other.allocator, nullptr)),
        buffer(std::exchange(other.buffer, nullptr)),
        allocation(std::exchange(other.allocation, nullptr)),
        size(std::exchange(other.size, 0)) {}

    MappedBuffer& MappedBuffer::operator=(MappedBuffer&& other) {
        device = std::exchange(other.device, nullptr);
        allocator = std::exchange(other.allocator, nullptr);
        buffer = std::exchange(other.buffer, nullptr);
        allocation = std::exchange(other.allocation, nullptr);
        size = std::exchange(other.size, 0);

        return *this;
    }

    VkBuffer MappedBuffer::get_buffer() const {
        return buffer;
    }

    void MappedBuffer::upload(const void* data, std::size_t size) const {
        if (size == 0) {
            return;
        }
        // TODO: Handle this correctly by expanding the buffer
        if (size > this->size) {
            throw std::runtime_error("Uploaded data (" + std::to_string(size) +
                ") exceeds buffer size (" + std::to_string(this->size) + ").");
        }
        // TODO: This can be simplified to vmaCopyMemoryToAllocation in VMA 3.1.0
        void* destination;
        vmaMapMemory(allocator->get_allocator(), allocation, &destination);
        std::memcpy(destination, data, size);
        vmaUnmapMemory(allocator->get_allocator(), allocation);
        vmaFlushAllocation(allocator->get_allocator(), allocation, 0, size);
    }

}