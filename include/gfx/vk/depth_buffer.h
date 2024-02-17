#pragma once

#include "gfx/vk/device.h"
#include "gfx/vk/image.h"

namespace inf::gfx::vk {

    struct DepthBuffer {

        static DepthBuffer create(
            const LogicalDevice* logical_device,
            const PhysicalDevice& physical_device,
            const VkExtent2D& swap_chain_extent,
            VkSampleCountFlagBits samples,
            bool is_sampled);

        DepthBuffer(Image&& image, ImageView&& image_view);

        const Image& get_image() const;
        const ImageView& get_image_view() const;

    private:

        Image image;
        ImageView image_view;

    };

}