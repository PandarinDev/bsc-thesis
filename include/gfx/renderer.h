#pragma once

#include "window.h"
#include "gfx/vk/instance.h"
#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"
#include "gfx/vk/pipeline.h"
#include "gfx/vk/framebuffer.h"
#include "gfx/vk/command.h"
#include "gfx/vk/semaphore.h"

#include <memory>
#include <vector>
#include <cstdint>
namespace inf::gfx {

    struct Renderer {

        Renderer(const Window& window);

        const vk::Instance& get_vulkan_instance() const;
        const vk::LogicalDevice& get_device() const;

        void begin_frame();
        void end_frame() const;

    private:

        std::uint32_t image_index;
        std::unique_ptr<vk::Instance> instance;
        std::unique_ptr<vk::Surface> surface;
        std::unique_ptr<vk::PhysicalDevice> physical_device;
        std::unique_ptr<vk::LogicalDevice> logical_device;
        std::unique_ptr<vk::SwapChain> swap_chain;
        std::vector<vk::Shader> shaders;
        std::unique_ptr<vk::RenderPass> render_pass;
        std::unique_ptr<vk::Pipeline> pipeline;
        std::vector<vk::Framebuffer> framebuffers;
        std::unique_ptr<vk::CommandPool> command_pool;
        std::unique_ptr<vk::CommandBuffer> command_buffer;
        std::unique_ptr<vk::Semaphore> image_available_semaphore;
        std::unique_ptr<vk::Semaphore> render_finished_semaphore;
        std::unique_ptr<vk::Fence> in_flight_fence;

    };

}