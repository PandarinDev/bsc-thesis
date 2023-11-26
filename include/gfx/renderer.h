#pragma once

#include "window.h"
#include "gfx/vk/instance.h"
#include "gfx/vk/device.h"

#include <memory>

namespace inf::gfx {

    struct Renderer {

        Renderer(const Window& window);

        const vk::Instance& get_vulkan_instance() const;

    private:

        std::unique_ptr<vk::Instance> instance;
        std::unique_ptr<vk::Surface> surface;
        std::unique_ptr<vk::PhysicalDevice> physical_device;
        std::unique_ptr<vk::LogicalDevice> logical_device;
        std::unique_ptr<vk::SwapChain> swap_chain;

    };

}