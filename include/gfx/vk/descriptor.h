#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct DescriptorSetLayout {

        static DescriptorSetLayout create(
            const LogicalDevice* device,
            std::uint32_t binding,
            ShaderType shader_type);

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

        std::vector<VkDescriptorSet> allocate_sets_for_buffers(
            const DescriptorSetLayout& layout,
            const std::vector<VkBuffer>& buffers) const;

    private:

        const LogicalDevice* device;
        VkDescriptorPool descriptor_pool;

    };

}