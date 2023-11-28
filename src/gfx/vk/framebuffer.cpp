#include "gfx/vk/framebuffer.h"

#include <utility>

namespace inf::gfx::vk {

    Framebuffer Framebuffer::create_from_image_view(
        const LogicalDevice* device,
        const RenderPass& render_pass,
        const VkImageView& image_view,
        const VkExtent2D& swap_chain_extent) {
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass.get_render_pass();
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &image_view;
        framebuffer_create_info.width = swap_chain_extent.width;
        framebuffer_create_info.height = swap_chain_extent.height;
        framebuffer_create_info.layers = 1;

        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(device->get_device(), &framebuffer_create_info, nullptr, &framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan framebuffer.");
        }
        return Framebuffer(device, framebuffer);
    }

    Framebuffer::Framebuffer(const LogicalDevice* device, const VkFramebuffer& framebuffer) :
        device(device),
        framebuffer(framebuffer) {}

    Framebuffer::~Framebuffer() {
        if (device) {
            vkDestroyFramebuffer(device->get_device(), framebuffer, nullptr);
        }
    }

    Framebuffer::Framebuffer(Framebuffer&& other) :
        device(std::exchange(other.device, nullptr)),
        framebuffer(std::exchange(other.framebuffer, VK_NULL_HANDLE)) {}

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) {
        device = std::exchange(other.device, nullptr);
        framebuffer = std::exchange(other.framebuffer, VK_NULL_HANDLE);

        return *this;
    }

    VkFramebuffer Framebuffer::get_framebuffer() const {
        return framebuffer;
    }

}