#include "gfx/vk/pipeline.h"
#include "gfx/vk/framebuffer.h"
#include "gfx/vk/command.h"
#include "gfx/vk/vertex.h"

#include <glm/matrix.hpp>

#include <array>
#include <vector>
#include <optional>
#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    RenderPass RenderPass::create_render_pass(
        const LogicalDevice* device,
        VkFormat swap_chain_format,
        VkSampleCountFlagBits samples) {
        // Create a color attachment
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swap_chain_format;
        color_attachment.samples = samples;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // If we are using multisampling the color attachment cannot be presented directly,
        // we need to attach a resolve attachment instead where the samples will be resolved.
        color_attachment.finalLayout = samples > VK_SAMPLE_COUNT_1_BIT
            ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Create a depth attachment
        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = VK_FORMAT_D32_SFLOAT;
        depth_attachment.samples = samples;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Create a resolve attachment if necessary because of multisampling
        std::optional<VkAttachmentDescription> maybe_resolve_attachment;
        std::optional<VkAttachmentReference> maybe_resolve_attachment_reference;
        if (samples > VK_SAMPLE_COUNT_1_BIT) {
            VkAttachmentDescription color_attachment_resolve{};
            color_attachment_resolve.format = swap_chain_format;
            color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            maybe_resolve_attachment = std::move(color_attachment_resolve);

            VkAttachmentReference resolve_attachment_reference{};
            resolve_attachment_reference.attachment = 2;
            resolve_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   
            maybe_resolve_attachment_reference = std::move(resolve_attachment_reference);
        }

        // Create subpass
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;
        subpass.pDepthStencilAttachment = &depth_attachment_reference;
        if (maybe_resolve_attachment_reference.has_value()) {
            subpass.pResolveAttachments = &maybe_resolve_attachment_reference.value();
        }

        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // If we are using multisampling the color data needs to be written to the multisampled
        // color attachment (0), so the color attachment needs to come first and the resolve
        // attachment needs to be last.
        std::vector<VkAttachmentDescription> attachments = maybe_resolve_attachment.has_value()
            ? std::vector<VkAttachmentDescription>{ color_attachment, depth_attachment, maybe_resolve_attachment.value() }
            : std::vector<VkAttachmentDescription>{ color_attachment, depth_attachment };
        // Create render pass
        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;

        VkRenderPass render_pass;
        if (vkCreateRenderPass(device->get_device(), &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass.");
        }
        return RenderPass(device, render_pass);
    }

    RenderPass RenderPass::create_shadow_render_pass(const LogicalDevice* device) {
        // Same as the regular depth attachment, except we want to store the results (and we don't do multisampling)
        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = VK_FORMAT_D32_SFLOAT;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        VkAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment = 0;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pDepthStencilAttachment = &depth_attachment_reference;

        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // Create render pass
        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &depth_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;

        VkRenderPass render_pass;
        if (vkCreateRenderPass(device->get_device(), &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass for shadow map.");
        }
        return RenderPass(device, render_pass);
    }

    RenderPass::RenderPass(const LogicalDevice* device, const VkRenderPass& render_pass) :
        device(device),
        render_pass(render_pass) {}

    RenderPass::~RenderPass() {
        if (device) {
            vkDestroyRenderPass(device->get_device(), render_pass, nullptr);
        }
    }

    RenderPass::RenderPass(RenderPass&& other) :
        device(std::exchange(other.device, nullptr)),
        render_pass(std::exchange(other.render_pass, VK_NULL_HANDLE)) {}

    RenderPass& RenderPass::operator=(RenderPass&& other) {
        device = std::exchange(other.device, nullptr);
        render_pass = std::exchange(other.render_pass, VK_NULL_HANDLE);
        
        return *this;
    }

    VkRenderPass RenderPass::get_render_pass() const {
        return render_pass;
    }

    void RenderPass::begin(
        const Framebuffer& framebuffer,
        const VkExtent2D& swap_chain_extent,
        const CommandBuffer& command_buffer,
        const std::vector<VkClearValue>& clear_values) const {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = framebuffer.get_framebuffer();
        render_pass_begin_info.renderArea.offset = { 0, 0 };
        render_pass_begin_info.renderArea.extent = swap_chain_extent;
        render_pass_begin_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(command_buffer.get_command_buffer(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::end(const CommandBuffer& command_buffer) const {
        vkCmdEndRenderPass(command_buffer.get_command_buffer());
    }

    Pipeline Pipeline::create_pipeline(
        const LogicalDevice* device,
        const RenderPass& render_pass,
        const VkExtent2D& swap_chain_extent,
        const DescriptorSetLayout& descriptor_set_layout,
        const std::vector<Shader>& shaders,
        VkSampleCountFlagBits samples,
        const std::optional<PipelineDepthBias>& depth_bias) {
        // Create shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
        for (const auto& shader : shaders) {
            VkPipelineShaderStageCreateInfo shader_stage_create_info{};
            shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(shader.get_type());
            shader_stage_create_info.module = shader.get_module();
            shader_stage_create_info.pName = "main";
            shader_stage_create_infos.emplace_back(std::move(shader_stage_create_info));
        }
        
        // Allocate space for push constants
        // TODO: This should be a function parameter (a vector of push constant ranges)
        VkPushConstantRange push_constant{};
        push_constant.offset = 0;
        push_constant.size = sizeof(glm::mat4) + sizeof(std::int32_t);
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // Create pipeline layout
        VkDescriptorSetLayout descriptor_set_layout_handle = descriptor_set_layout.get_descriptor_set_layout();
        VkPipelineLayoutCreateInfo layout_create_info{};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 1;
        layout_create_info.pSetLayouts = &descriptor_set_layout_handle;
        layout_create_info.pushConstantRangeCount = 1;
        layout_create_info.pPushConstantRanges = &push_constant;

        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(device->get_device(), &layout_create_info, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan pipeline layout.");
        }

        // Vertex input and assembly info creation
        const auto vertex_binding_description = Vertex::get_binding_descriptions();
        const auto vertex_attribute_descriptions = Vertex::get_attribute_descriptions();
        VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
        vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_create_info.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_binding_description.size());
        vertex_input_create_info.pVertexBindingDescriptions = vertex_binding_description.data();
        vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_attribute_descriptions.size());
        vertex_input_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        // Viewport
        VkViewport viewport{};
        viewport.width = static_cast<float>(swap_chain_extent.width);
        viewport.height = static_cast<float>(swap_chain_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = swap_chain_extent;

        VkPipelineViewportStateCreateInfo viewport_create_info{};
        viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo raster_create_info{};
        raster_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster_create_info.depthClampEnable = VK_FALSE;
        raster_create_info.rasterizerDiscardEnable = VK_FALSE;
        raster_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        raster_create_info.lineWidth = 1.0f;
        raster_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        raster_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster_create_info.depthBiasEnable = depth_bias ? VK_TRUE : VK_FALSE;
        raster_create_info.depthBiasConstantFactor = depth_bias ? depth_bias->constant_factor : 0.0f;
        raster_create_info.depthBiasSlopeFactor = depth_bias ? depth_bias->slope_factor : 0.0f;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisample_create_info{};
        multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_create_info.sampleShadingEnable = VK_FALSE;
        multisample_create_info.rasterizationSamples = samples;

        // Color blending
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
        color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_create_info.logicOpEnable = VK_FALSE;
        color_blend_create_info.attachmentCount = 1;
        color_blend_create_info.pAttachments = &color_blend_attachment;

        // Dynamic state data
        VkDynamicState dynamic_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        // Depth & stencil state
        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
        depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable = VK_TRUE;
        depth_stencil_create_info.depthWriteEnable = VK_TRUE;
        depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable = VK_FALSE;

        // Create graphics pipeline
        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = static_cast<std::uint32_t>(shaders.size());
        pipeline_create_info.pStages = shader_stage_create_infos.data();
        pipeline_create_info.pVertexInputState = &vertex_input_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_create_info.pViewportState = &viewport_create_info;
        pipeline_create_info.pRasterizationState = &raster_create_info;
        pipeline_create_info.pMultisampleState = &multisample_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_create_info;
        pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        pipeline_create_info.layout = layout;
        pipeline_create_info.renderPass = render_pass.get_render_pass();
        pipeline_create_info.subpass = 0;
        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(device->get_device(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan graphics pipeline.");
        }
        return Pipeline(device, layout, pipeline);
    }

    Pipeline::Pipeline(const LogicalDevice* device, const VkPipelineLayout& layout, const VkPipeline& pipeline) :
        device(device),
        layout(layout),
        pipeline(pipeline) {}

    Pipeline::~Pipeline() {
        if (device) {
            vkDestroyPipeline(device->get_device(), pipeline, nullptr);
            vkDestroyPipelineLayout(device->get_device(), layout, nullptr);
        }
    }

    Pipeline::Pipeline(Pipeline&& other) :
        device(std::exchange(other.device, nullptr)),
        layout(std::exchange(other.layout, VK_NULL_HANDLE)),
        pipeline(std::exchange(other.pipeline, VK_NULL_HANDLE)) {}

    Pipeline& Pipeline::operator=(Pipeline&& other) {
        device = std::exchange(other.device, nullptr);
        layout = std::exchange(other.layout, VK_NULL_HANDLE);
        pipeline = std::exchange(other.pipeline, VK_NULL_HANDLE);

        return *this;
    }

    VkPipeline Pipeline::get_pipeline() const {
        return pipeline;
    }

    VkPipelineLayout Pipeline::get_pipeline_layout() const {
        return layout;
    }

}