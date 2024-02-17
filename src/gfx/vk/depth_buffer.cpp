#include "gfx/vk/depth_buffer.h"

namespace inf::gfx::vk {

    DepthBuffer DepthBuffer::create(
        const LogicalDevice* logical_device,
        const PhysicalDevice& physical_device,
        const VkExtent2D& swap_chain_extent,
        VkSampleCountFlagBits samples,
        bool is_sampled) {
        static constexpr auto depth_format = VK_FORMAT_D32_SFLOAT;
        auto image = Image::create(
            logical_device,
            physical_device,
            swap_chain_extent.width,
            swap_chain_extent.height,
            depth_format,
            VK_IMAGE_TILING_OPTIMAL,
            is_sampled
                ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            samples);
        auto image_view = ImageView::create(
            logical_device,
            image.get_image(),
            depth_format,
            VK_IMAGE_ASPECT_DEPTH_BIT);
        return DepthBuffer(std::move(image), std::move(image_view));
    }

    DepthBuffer::DepthBuffer(Image&& image, ImageView&& image_view) :
        image(std::move(image)),
        image_view(std::move(image_view)) {}

    const Image& DepthBuffer::get_image() const {
        return image;
    }

    const ImageView& DepthBuffer::get_image_view() const {
        return image_view;
    }

}