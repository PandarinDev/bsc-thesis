#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"
#include "gfx/vk/descriptor.h"

#include <glad/vulkan.h>

#include <vector>

namespace inf::gfx::vk {

    struct Framebuffer;
    struct CommandBuffer;

    struct RenderPass {

        static RenderPass create_render_pass(const LogicalDevice* device, VkFormat swap_chain_format);

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
            const CommandBuffer& command_buffer) const;
        void end(const CommandBuffer& command_buffer) const;

    private:

        const LogicalDevice* device;
        VkRenderPass render_pass;

    };

    struct Pipeline {

        static Pipeline create_pipeline(
            const LogicalDevice* device,
            const RenderPass& render_pass,
            const VkExtent2D& swap_chain_extent,
            const DescriptorSetLayout& descriptor_set_layout,
            const std::vector<Shader>& shaders);
        
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