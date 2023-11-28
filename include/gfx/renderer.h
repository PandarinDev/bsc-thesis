#pragma once

#include "window.h"
#include "gfx/vk/instance.h"
#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"
#include "gfx/vk/pipeline.h"

#include <memory>
#include <vector>

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
        std::vector<vk::Shader> shaders;
        std::unique_ptr<vk::RenderPass> render_pass;
        std::unique_ptr<vk::Pipeline> pipeline;

    };

}