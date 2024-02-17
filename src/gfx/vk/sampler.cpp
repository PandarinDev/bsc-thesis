#include "gfx/vk/sampler.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    Sampler Sampler::create(const LogicalDevice* logical_device) {
        VkSamplerCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.magFilter = VK_FILTER_NEAREST;
        create_info.minFilter = VK_FILTER_NEAREST;
        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_info.maxAnisotropy = 1.0f;
        create_info.maxLod = 1.0f;
        create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VkSampler sampler;
        if (vkCreateSampler(logical_device->get_device(), &create_info, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan sampler.");
        }
        return Sampler(logical_device, sampler);
    }

    Sampler::Sampler(const LogicalDevice* device, VkSampler sampler) :
        device(device), sampler(sampler) {}

    Sampler::~Sampler() {
        if (device) {
            vkDestroySampler(device->get_device(), sampler, nullptr);
        }
    }

    Sampler::Sampler(Sampler&& other) :
        device(std::exchange(other.device, nullptr)),
        sampler(std::exchange(other.sampler, nullptr)) {}

    Sampler& Sampler::operator=(Sampler&& other) {
        device = std::exchange(other.device, nullptr);
        sampler = std::exchange(other.sampler, nullptr);
        return *this;
    }

    VkSampler Sampler::get_sampler() const {
        return sampler;
    }

}