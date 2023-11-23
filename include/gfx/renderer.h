#pragma once

#include "gfx/vk/instance.h"
#include "gfx/vk/device.h"

#include <memory>

namespace inf::gfx {

    struct Renderer {

        Renderer();

    private:

        std::unique_ptr<vk::Instance> instance;
        VkPhysicalDevice device;

    };

}