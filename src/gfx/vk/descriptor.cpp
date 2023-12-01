#include "gfx/vk/descriptor.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    DescriptorSetLayout DescriptorSetLayout::create(
        const LogicalDevice* device,
        std::uint32_t binding,
        ShaderType shader_type) {
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = binding;
        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = static_cast<VkShaderStageFlags>(shader_type);

        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount = 1;
        layout_create_info.pBindings = &layout_binding;

        VkDescriptorSetLayout descriptor_set_layout;
        if (vkCreateDescriptorSetLayout(device->get_device(), &layout_create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan descriptor set layout.");
        }
        return DescriptorSetLayout(device, descriptor_set_layout);
    }

    DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice* device, const VkDescriptorSetLayout& descriptor_set_layout) :
        device(device),
        descriptor_set_layout(descriptor_set_layout) {}

    DescriptorSetLayout::~DescriptorSetLayout() {
        if (device) {
            vkDestroyDescriptorSetLayout(device->get_device(), descriptor_set_layout, nullptr);
        }
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
        device(std::exchange(other.device, nullptr)),
        descriptor_set_layout(std::exchange(other.descriptor_set_layout, VK_NULL_HANDLE)) {}

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) {
        device = std::exchange(other.device, nullptr);
        descriptor_set_layout = std::exchange(other.descriptor_set_layout, VK_NULL_HANDLE);
        return *this;
    }

    VkDescriptorSetLayout DescriptorSetLayout::get_descriptor_set_layout() const {
        return descriptor_set_layout;
    }

    DescriptorPool DescriptorPool::create(const LogicalDevice* device, std::uint32_t size) {
        VkDescriptorPoolSize pool_size{};
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_size.descriptorCount = size;

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount = 1;
        pool_create_info.pPoolSizes = &pool_size;
        pool_create_info.maxSets = size;

        VkDescriptorPool descriptor_pool;
        if (vkCreateDescriptorPool(device->get_device(), &pool_create_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan descriptor pool.");
        }
        return DescriptorPool(device, descriptor_pool);
    }

    DescriptorPool::DescriptorPool(const LogicalDevice* device, const VkDescriptorPool& descriptor_pool) :
        device(device),
        descriptor_pool(descriptor_pool) {}

    DescriptorPool::~DescriptorPool() {
        if (device) {
            vkDestroyDescriptorPool(device->get_device(), descriptor_pool, nullptr);
        }
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) :
        device(std::exchange(other.device, nullptr)),
        descriptor_pool(std::exchange(other.descriptor_pool, VK_NULL_HANDLE)) {}

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) {
        device = std::exchange(other.device, nullptr);
        descriptor_pool = std::exchange(other.descriptor_pool, VK_NULL_HANDLE);

        return *this;
    }

    VkDescriptorPool DescriptorPool::get_descriptor_pool() const {
        return descriptor_pool;
    }

    std::vector<VkDescriptorSet> DescriptorPool::allocate_sets_for_buffers(
        const DescriptorSetLayout& layout,
        const std::vector<VkBuffer>& buffers) const {
        std::vector<VkDescriptorSetLayout> layout_handles(buffers.size(), layout.get_descriptor_set_layout());
        VkDescriptorSetAllocateInfo set_allocate_info{};
        set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_allocate_info.descriptorPool = descriptor_pool;
        set_allocate_info.descriptorSetCount = static_cast<std::uint32_t>(buffers.size());
        set_allocate_info.pSetLayouts = layout_handles.data();

        std::vector<VkDescriptorSet> descriptor_sets(buffers.size());
        if (vkAllocateDescriptorSets(device->get_device(), &set_allocate_info, descriptor_sets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan descriptor sets.");
        }

        // Update descriptors to point to their respective buffer
        for (std::size_t i = 0; i < buffers.size(); ++i) {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE;

            VkWriteDescriptorSet write_descriptor_set{};
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.dstSet = descriptor_sets[i];
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(device->get_device(), 1, &write_descriptor_set, 0, nullptr);
        }

        return descriptor_sets;
    }

}