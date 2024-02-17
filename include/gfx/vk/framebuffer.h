#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/pipeline.h"
#include "gfx/vk/image.h"

#include <glad/vulkan.h>

namespace inf::gfx::vk {

    struct Framebuffer {

        static Framebuffer create_from_image_view(
            const LogicalDevice* device,
            const RenderPass& render_pass,
            const ImageView& image_view,
            const ImageView& depth_image_view,
            const ImageView* color_image_view,
            const VkExtent2D& swap_chain_extent);
        static Framebuffer create_for_shadow_map(
            const LogicalDevice* device,
            const RenderPass& render_pass,
            const ImageView& image_view,
            const VkExtent2D& extent);

        Framebuffer(const LogicalDevice* device, const VkFramebuffer& framebuffer);
        ~Framebuffer();
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&&);
        Framebuffer& operator=(Framebuffer&&);

        VkFramebuffer get_framebuffer() const;

    private:

        const LogicalDevice* device;
        VkFramebuffer framebuffer;

    };

}