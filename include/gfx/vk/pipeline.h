#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"
#include "gfx/vk/descriptor.h"

#include <glad/vulkan.h>

#include <vector>
#include <optional>

namespace inf::gfx::vk {

    struct Framebuffer;
    struct CommandBuffer;

    struct RenderPass {

        static RenderPass create_render_pass(
            const LogicalDevice* device,
            VkFormat swap_chain_format,
            VkSampleCountFlagBits samples);
        static RenderPass create_shadow_render_pass(const LogicalDevice* device);

        RenderPass(const LogicalDevice* device, const VkRenderPass& render_pass);
        ~RenderPass();
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass(RenderPass&&);
        RenderPass& operator=(RenderPass&&);

        VkRenderPass get_render_pass() const;

        void begin(
            const Framebuffer& framebuffer,
            const VkExtent2D& swap_chain_extent,
            const CommandBuffer& command_buffer,
            const std::vector<VkClearValue>& clear_values) const;
        void end(const CommandBuffer& command_buffer) const;

    private:

        const LogicalDevice* device;
        VkRenderPass render_pass;

    };

    struct PipelineDepthBias {
        float constant_factor;
        float slope_factor;
    };

    struct Pipeline {

        static Pipeline create_pipeline(
            const LogicalDevice* device,
            const RenderPass& render_pass,
            const VkExtent2D& swap_chain_extent,
            const DescriptorSetLayout& descriptor_set_layout,
            const std::vector<Shader>& shaders,
            std::uint32_t num_binding_descriptions,
            const VkVertexInputBindingDescription* binding_descriptions,
            std::uint32_t num_attribute_descriptions,
            const VkVertexInputAttributeDescription* attribute_descriptions,
            VkSampleCountFlagBits samples,
            const std::optional<PipelineDepthBias>& depth_bias);
        
        Pipeline(const LogicalDevice* device, const VkPipelineLayout& layout, const VkPipeline& pipeline);
        ~Pipeline();
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline(Pipeline&&);
        Pipeline& operator=(Pipeline&&);

        VkPipeline get_pipeline() const;
        VkPipelineLayout get_pipeline_layout() const;

    private:

        const LogicalDevice* device;
        VkPipelineLayout layout;
        VkPipeline pipeline;

    };

}