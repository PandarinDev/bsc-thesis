#pragma once

#include "window.h"
#include "camera.h"
#include "gfx/geometry.h"
#include "gfx/vk/instance.h"
#include "gfx/vk/device.h"
#include "gfx/vk/shader.h"
#include "gfx/vk/pipeline.h"
#include "gfx/vk/framebuffer.h"
#include "gfx/vk/command.h"
#include "gfx/vk/semaphore.h"
#include "gfx/vk/descriptor.h"
#include "gfx/vk/buffer.h"
#include "gfx/vk/depth_buffer.h"
#include "gfx/mesh.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <cstdint>

namespace inf::gfx {

    struct Matrices {
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
    };

    struct Renderer {

        static constexpr std::uint8_t MAX_FRAMES_IN_FLIGHT = 2;
        static constexpr float FOVY = glm::radians(65.0f);
        static constexpr float NEAR_PLANE = 0.01f;
        static constexpr float FAR_PLANE = 100.0f;

        Renderer(const Window& window, const Camera& camera);

        const vk::Instance& get_vulkan_instance() const;
        const vk::PhysicalDevice& get_physical_device() const;
        const vk::LogicalDevice& get_logical_device() const;

        void begin_frame();
        void render(const Mesh& mesh) const;
        void end_frame();

    private:

        const Camera& camera;
        std::uint32_t image_index;
        std::uint8_t frame_index;
        std::unique_ptr<vk::Instance> instance;
        std::unique_ptr<vk::Surface> surface;
        std::unique_ptr<vk::PhysicalDevice> physical_device;
        std::unique_ptr<vk::LogicalDevice> logical_device;
        std::unique_ptr<vk::SwapChain> swap_chain;
        std::vector<vk::Shader> shaders;
        std::unique_ptr<vk::DescriptorPool> descriptor_pool;
        std::unique_ptr<vk::DescriptorSetLayout> descriptor_set_layout;
        std::vector<VkDescriptorSet> descriptor_sets;
        std::unique_ptr<vk::RenderPass> render_pass;
        std::unique_ptr<vk::Pipeline> pipeline;
        std::unique_ptr<vk::Image> color_image;
        std::unique_ptr<vk::ImageView> color_image_view;
        std::unique_ptr<vk::DepthBuffer> depth_buffer;
        std::vector<vk::Framebuffer> framebuffers;
        std::unique_ptr<vk::CommandPool> command_pool;
        std::vector<vk::CommandBuffer> command_buffers;
        std::vector<vk::Semaphore> image_available_semaphores;
        std::vector<vk::Semaphore> render_finished_semaphores;
        std::vector<vk::Fence> in_flight_fences;
        std::vector<vk::MappedBuffer> uniform_buffers;
        glm::mat4 projection_matrix;

    };

}