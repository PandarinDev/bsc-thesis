#pragma once

#include "gfx/vk/device.h"

namespace inf::gfx::vk {

    struct Sampler {

        static Sampler create(const LogicalDevice* logical_device);

        Sampler(const LogicalDevice* device, VkSampler sampler);
        ~Sampler();
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&&);
        Sampler& operator=(Sampler&&);

        VkSampler get_sampler() const;

    private:

        const LogicalDevice* device;
        VkSampler sampler;

    };

}