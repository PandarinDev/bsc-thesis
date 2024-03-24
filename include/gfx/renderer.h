#pragma once

// In this particular case we want to make sure GLM is included first, to ensure depth [0; 1] is applied
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "window.h"
#include "camera.h"
#include "timer.h"
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
#include "gfx/vk/sampler.h"
#include "gfx/vk/memory_allocator.h"
#include "gfx/mesh.h"
#include "bounding_box.h"

#include <memory>
#include <vector>
#include <cstdint>

namespace inf::gfx {

    struct Matrices {
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 light_space_matrix;
    };

    struct Renderer {

        static constexpr std::uint8_t MAX_FRAMES_IN_FLIGHT = 2;
        static constexpr float FOVY = glm::radians(65.0f);
        static constexpr float NEAR_PLANE = 0.01f;
        static constexpr float FAR_PLANE = 100.0f;

        Renderer(const Window& window, const Camera& camera, const Timer& timer);

        const Camera& get_camera() const;
        const vk::Instance& get_vulkan_instance() const;
        const vk::PhysicalDevice& get_physical_device() const;
        const vk::LogicalDevice& get_logical_device() const;
        const vk::MemoryAllocator& get_memory_allocator() const;
        void set_show_diagnostics(bool show);

        void begin_frame(std::size_t num_buildings);
        void render(const Mesh& mesh);
        void render_instanced(const Mesh& mesh, std::vector<glm::vec3>&& positions);
        void end_frame();

        bool is_in_view(const BoundingBox3D& bounding_box) const;
        const glm::mat4& get_projection_matrix() const;
        glm::mat4 get_view_matrix() const;

        void destroy_imgui();

    private:

        // Because when we render the same mesh multiple times we modify the model matrix
        // in between render calls, so the model matrix needs to be stored at the time of
        // the render call.
        // TODO: This would be solved (along with better performance) by moving to instanced rendering.
        struct MeshToRender {
            const Mesh* mesh;
            const glm::mat4 model_matrix;
        };

        struct InstancedMeshToRender {
            const Mesh* mesh;
            std::vector<glm::vec3> positions;
        };

        const Camera& camera;
        const Timer& timer;
        std::uint32_t image_index;
        std::uint8_t frame_index;
        std::unique_ptr<vk::Instance> instance;
        std::unique_ptr<vk::Surface> surface;
        std::unique_ptr<vk::PhysicalDevice> physical_device;
        std::unique_ptr<vk::LogicalDevice> logical_device;
        std::unique_ptr<vk::MemoryAllocator> memory_allocator;
        std::unique_ptr<vk::SwapChain> swap_chain;
        std::vector<vk::Shader> shaders;
        std::vector<vk::Shader> instanced_shaders;
        std::vector<vk::Shader> shadow_map_shaders;
        std::unique_ptr<vk::DescriptorPool> descriptor_pool;
        std::unique_ptr<vk::DescriptorSetLayout> descriptor_set_layout;
        std::unique_ptr<vk::DescriptorSetLayout> instanced_descriptor_set_layout;
        std::unique_ptr<vk::DescriptorSetLayout> shadow_map_descriptor_set_layout;
        std::vector<VkDescriptorSet> descriptor_sets;
        VkDescriptorSet shadow_map_descriptor_set;
        std::unique_ptr<vk::RenderPass> render_pass;
        std::unique_ptr<vk::RenderPass> shadow_map_render_pass;
        std::unique_ptr<vk::Pipeline> pipeline;
        std::unique_ptr<vk::Pipeline> instanced_pipeline;
        std::unique_ptr<vk::Pipeline> shadow_map_pipeline;
        std::unique_ptr<vk::Image> color_image;
        std::unique_ptr<vk::ImageView> color_image_view;
        std::unique_ptr<vk::DepthBuffer> depth_buffer;
        std::unique_ptr<vk::DepthBuffer> shadow_map_depth_buffer;
        std::vector<vk::Framebuffer> framebuffers;
        std::unique_ptr<vk::Framebuffer> shadow_map_framebuffer;
        std::unique_ptr<vk::Sampler> shadow_map_sampler;
        std::unique_ptr<vk::CommandPool> command_pool;
        std::vector<vk::CommandBuffer> command_buffers;
        std::vector<vk::Semaphore> image_available_semaphores;
        std::vector<vk::Semaphore> render_finished_semaphores;
        std::vector<vk::Fence> in_flight_fences;
        std::vector<vk::MappedBuffer> uniform_buffers;
        std::unique_ptr<vk::MappedBuffer> shadow_map_uniform_buffer;
        glm::mat4 projection_matrix;
        glm::mat4 shadow_map_projection_matrix;
        std::vector<MeshToRender> shadow_casters_to_render;
        std::vector<InstancedMeshToRender> non_casters_to_render;
        bool show_diagnostics;

        void init_imgui(const Window& window, VkSampleCountFlagBits sample_count);

    };

}