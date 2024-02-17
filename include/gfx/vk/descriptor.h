#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"

#include <glad/vulkan.h>

#include <vector>

namespace inf::gfx::vk {

    struct DescriptorSetLayout {

        static DescriptorSetLayout create(
            const LogicalDevice* device,
            const std::vector<VkDescriptorSetLayoutBinding>& bindings);

        DescriptorSetLayout(const LogicalDevice* device, const VkDescriptorSetLayout& descriptor_set_layout);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&&);
        DescriptorSetLayout& operator=(DescriptorSetLayout&&);

        VkDescriptorSetLayout get_descriptor_set_layout() const;

    private:

        const LogicalDevice* device;
        VkDescriptorSetLayout descriptor_set_layout;

    };

    struct DescriptorPool {

        static DescriptorPool create(const LogicalDevice* device, std::uint32_t size);

        DescriptorPool(const LogicalDevice* device, const VkDescriptorPool& descriptor_pool);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&&);
        DescriptorPool& operator=(DescriptorPool&&);

        VkDescriptorPool get_descriptor_pool() const;

        std::vector<VkDescriptorSet> allocate_sets(
            const DescriptorSetLayout& layout,
            std::vector<std::vector<VkWriteDescriptorSet>>& write_descriptor_sets,
            std::uint32_t number_of_sets_to_create) const;

    private:

        const LogicalDevice* device;
        VkDescriptorPool descriptor_pool;

    };

    struct WriteDescriptorSet {


        static VkWriteDescriptorSet create_for_buffer(
            const VkDescriptorBufferInfo& buffer_info,
            std::uint32_t binding);

        static VkWriteDescriptorSet create_for_sampler(
            const VkDescriptorImageInfo& image_info,
            std::uint32_t binding);
            
        WriteDescriptorSet() = delete;

    };

}