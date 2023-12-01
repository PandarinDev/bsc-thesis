#include "gfx/vk/buffer.h"

#include <utility>
#include <stdexcept>
#include <optional>

namespace inf::gfx::vk {

    MappedBuffer MappedBuffer::create(
        const PhysicalDevice& physical_device,
        const LogicalDevice* logical_device,
        BufferType type,
        std::uint64_t size) {
        // Create the buffer
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size;
        buffer_create_info.usage = static_cast<VkBufferUsageFlags>(type);
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer;
        if (vkCreateBuffer(logical_device->get_device(), &buffer_create_info, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan buffer.");
        }

        // Query buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical_device->get_device(), buffer, &memory_requirements);

        // Find an appropriate memory type that fulfills the buffer requirements
        const auto memory_properties = physical_device.query_memory_properties();
        std::optional<std::uint32_t> memory_type_index;
        for (std::uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
            // First check if the memory type is supported for this type of buffer
            if (!(memory_requirements.memoryTypeBits & (1 << i))) {
                continue;
            }
            // Then check if the memory type supports the required properties, which in our case
            // for a mapped buffer would be that the memory is host visible and host coherent
            static constexpr auto memory_requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            if ((memory_properties.memoryTypes[i].propertyFlags & memory_requirements) == memory_requirements) {
                memory_type_index = i;
                break;
            }
        }
        if (!memory_type_index.has_value()) {
            throw std::runtime_error("No suitable memory found for Vulkan mapped buffer.");
        }

        // Allocate the device memory
        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = memory_type_index.value();

        VkDeviceMemory device_memory;
        if (vkAllocateMemory(logical_device->get_device(), &allocate_info, nullptr, &device_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan mapped buffer.");
        }

        // Bind the device memory to the buffer
        if (vkBindBufferMemory(logical_device->get_device(), buffer, device_memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Failed to bind Vulkan device memory to a mapped buffer.");
        }

        // Finally map the memory
        void* data;
        if (vkMapMemory(logical_device->get_device(), device_memory, 0, size, 0, &data) != VK_SUCCESS) {
            throw std::runtime_error("Failed to map Vulkan device memory to host memory.");
        }

        return MappedBuffer(logical_device, buffer, device_memory, data);
    }

    MappedBuffer::MappedBuffer(
        const LogicalDevice* device,
        const VkBuffer& buffer,
        const VkDeviceMemory& device_memory,
        void* data) :
        device(device),
        buffer(buffer),
        device_memory(device_memory),
        data(data) {}

    MappedBuffer::~MappedBuffer() {
        if (device) {
            vkDestroyBuffer(device->get_device(), buffer, nullptr);
            // We do not need to unmap the memory as it is done implicitly when freeing it
            vkFreeMemory(device->get_device(), device_memory, nullptr);
        }
    }

    MappedBuffer::MappedBuffer(MappedBuffer&& other) :
        device(std::exchange(other.device, nullptr)),
        buffer(std::exchange(other.buffer, VK_NULL_HANDLE)),
        device_memory(std::exchange(other.device_memory, VK_NULL_HANDLE)),
        data(std::exchange(other.data, nullptr)) {}

    MappedBuffer& MappedBuffer::operator=(MappedBuffer&& other) {
        device = std::exchange(other.device, nullptr);
        buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
        device_memory = std::exchange(other.device_memory, VK_NULL_HANDLE);
        data = std::exchange(other.data, nullptr);

        return *this;
    }

    VkBuffer MappedBuffer::get_buffer() const {
        return buffer;
    }

}