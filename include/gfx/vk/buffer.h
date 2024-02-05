#pragma once

#include "gfx/vk/device.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    enum class BufferType {
        UNIFORM_BUFFER = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VERTEX_BUFFER = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    struct MappedBuffer {

        static MappedBuffer create(
            const PhysicalDevice& physical_device,
            const LogicalDevice* logical_device,
            BufferType type,
            std::uint64_t size);

        MappedBuffer(
            const LogicalDevice* device,
            const VkBuffer& buffer,
            const VkDeviceMemory& device_memory,
            void* data);
        ~MappedBuffer();
        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer& operator=(const MappedBuffer&) = delete;
        MappedBuffer(MappedBuffer&&);
        MappedBuffer& operator=(MappedBuffer&&);

        VkBuffer get_buffer() const;

        template<typename T>
        void upload(const T& value) const {
            std::memcpy(data, &value, sizeof(value));
        }

        template<typename T>
        void upload(const std::vector<T>& vec) const {
            std::memcpy(data, vec.data(), vec.size() * sizeof(T));
        }

        void upload(const void* data, std::size_t size) const;

    private:

        const LogicalDevice* device;
        VkBuffer buffer;
        VkDeviceMemory device_memory;
        void* data;

    };

}