#include "gfx/vk/descriptor.h"

#include <array>
#include <string>
#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    DescriptorSetLayout DescriptorSetLayout::create(
        const LogicalDevice* device,
        const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
        layout_create_info.pBindings = bindings.data();

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
        std::array<VkDescriptorPoolSize, 2> pool_sizes;
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = size;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = size;

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
        pool_create_info.pPoolSizes = pool_sizes.data();
        pool_create_info.maxSets = size;
        pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

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

    std::vector<VkDescriptorSet> DescriptorPool::allocate_sets(
            const DescriptorSetLayout& layout,
            std::vector<std::vector<VkWriteDescriptorSet>>& write_descriptor_sets,
            std::uint32_t number_of_sets_to_create) const {
        // Each set uses the same layout
        std::vector<VkDescriptorSetLayout> layout_handles(number_of_sets_to_create, layout.get_descriptor_set_layout());
        VkDescriptorSetAllocateInfo set_allocate_info{};
        set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_allocate_info.descriptorPool = descriptor_pool;
        set_allocate_info.descriptorSetCount = number_of_sets_to_create;
        set_allocate_info.pSetLayouts = layout_handles.data();

        std::vector<VkDescriptorSet> descriptor_sets(number_of_sets_to_create);
        if (const auto result = vkAllocateDescriptorSets(
            device->get_device(), &set_allocate_info, descriptor_sets.data()); result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan descriptor sets: " + std::to_string(result));
        }

        // Update descriptors to point to their respective buffer
        for (std::size_t i = 0; i < number_of_sets_to_create; ++i) {
            // Update write descriptor sets to point to the correct destination
            for (auto& write_descriptor : write_descriptor_sets[i]) {
                write_descriptor.dstSet = descriptor_sets[i];
            }

            vkUpdateDescriptorSets(
                device->get_device(),
                static_cast<std::uint32_t>(write_descriptor_sets[i].size()),
                write_descriptor_sets[i].data(),
                0, nullptr);
        }

        return descriptor_sets;
    }

    VkWriteDescriptorSet WriteDescriptorSet::create_for_buffer(
        const VkDescriptorBufferInfo& buffer_info,
        std::uint32_t binding) {
        VkWriteDescriptorSet write_descriptor{};
        write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // Important: dstSet will be configured by allocateSets()
        write_descriptor.dstBinding = binding;
        write_descriptor.dstArrayElement = 0;
        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor.descriptorCount = 1;
        write_descriptor.pBufferInfo = &buffer_info;

        return write_descriptor;
    }

    VkWriteDescriptorSet WriteDescriptorSet::create_for_sampler(
        const VkDescriptorImageInfo& image_info,
        std::uint32_t binding) {
        VkWriteDescriptorSet write_descriptor{};
        write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // Important: dstSet will be configured by allocateSets()
        write_descriptor.dstBinding = binding;
        write_descriptor.dstArrayElement = 0;
        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor.descriptorCount = 1;
        write_descriptor.pImageInfo = &image_info;

        return write_descriptor;
    }

}