#pragma once

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "context.h"
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
#include "frustum.h"

#include <memory>
#include <vector>
#include <cstdint>

namespace inf::gfx {

    struct Matrices {
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 light_space_matrix;
        glm::vec3 light_direction;
        float ambient_light;
    };

    struct PushConstants {
        glm::mat4 model_matrix;
        std::int32_t debug_bb; // Boolean, but GLSL bools are 4 bytes
    };

    struct ParticleMatrices {
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 inverse_view_matrix;
        float ambient_light;
    };

    struct Renderer {

        static constexpr std::uint8_t MAX_FRAMES_IN_FLIGHT = 2;
        static constexpr float FOVY = glm::radians(65.0f);
        static constexpr float NEAR_PLANE = 0.01f;
        static constexpr float FAR_PLANE = 100.0f;

        Renderer(Context& context, const Window& window, const Camera& camera, const Timer& timer);

        const Camera& get_camera() const;
        const vk::Instance& get_vulkan_instance() const;
        const vk::PhysicalDevice& get_physical_device() const;
        const vk::LogicalDevice& get_logical_device() const;
        const vk::MemoryAllocator& get_memory_allocator() const;
        void set_show_diagnostics(bool show);

        void begin_frame(std::size_t num_districts, std::size_t num_buildings);
        void render(const Mesh& mesh);
        void render_instanced(const Mesh& mesh, const std::vector<glm::vec3>& positions, const std::vector<float>& rotations);
        void render_instanced_caster(const Mesh& mesh, const std::vector<glm::vec3>& positions, const std::vector<float>& rotations);
        void render_particles(const Mesh& mesh, const std::vector<glm::vec3>& positions);
        void render(const BoundingBox3D& bounding_box, const glm::vec3& color);
        void end_frame();

        bool is_in_view(const BoundingBox3D& bounding_box) const;
        const glm::mat4& get_projection_matrix() const;
        glm::mat4 get_view_matrix() const;
        Frustum get_frustum_in_view_space() const;
        Frustum get_frustum_in_world_space() const;

        void destroy_imgui();

    private:

        struct InstancedMeshToRender {
            const Mesh* mesh;
            const std::vector<glm::vec3>& positions;
            const std::vector<float>& rotations;
        };

        struct ParticlesToRender {
            const Mesh* mesh;
            const std::vector<glm::vec3>& positions;
        };

        Context& context;
        const Camera& camera;
        const Timer& timer;
        std::uint32_t image_index;
        std::uint8_t frame_index;

        // Vulkan objects
        std::unique_ptr<vk::Instance> instance;
        std::unique_ptr<vk::Surface> surface;
        std::unique_ptr<vk::PhysicalDevice> physical_device;
        std::unique_ptr<vk::LogicalDevice> logical_device;
        std::unique_ptr<vk::MemoryAllocator> memory_allocator;
        std::unique_ptr<vk::SwapChain> swap_chain;

        // Shaders and descriptor sets
        std::vector<vk::Shader> shaders;
        std::vector<vk::Shader> instanced_shaders;
        std::vector<vk::Shader> shadow_map_shaders;
        std::vector<vk::Shader> shadow_map_instanced_shaders;
        std::vector<vk::Shader> particle_shaders;
        std::unique_ptr<vk::DescriptorPool> descriptor_pool;
        std::unique_ptr<vk::DescriptorSetLayout> descriptor_set_layout;
        std::unique_ptr<vk::DescriptorSetLayout> instanced_descriptor_set_layout;
        std::unique_ptr<vk::DescriptorSetLayout> shadow_map_descriptor_set_layout;
        std::unique_ptr<vk::DescriptorSetLayout> particle_descriptor_set_layout;
        std::vector<VkDescriptorSet> descriptor_sets;
        std::vector<VkDescriptorSet> shadow_map_descriptor_sets;
        std::vector<VkDescriptorSet> particle_descriptor_sets;

        // Render passes and pipelines
        std::unique_ptr<vk::RenderPass> render_pass;
        std::unique_ptr<vk::RenderPass> shadow_map_render_pass;
        std::unique_ptr<vk::Pipeline> pipeline;
        std::unique_ptr<vk::Pipeline> instanced_pipeline;
        std::unique_ptr<vk::Pipeline> shadow_map_pipeline;
        std::unique_ptr<vk::Pipeline> shadow_map_instanced_pipeline;
        std::unique_ptr<vk::Pipeline> particle_pipeline;

        // Images, frame buffers, samplers
        std::unique_ptr<vk::Image> color_image;
        std::unique_ptr<vk::ImageView> color_image_view;
        std::unique_ptr<vk::DepthBuffer> depth_buffer;
        std::unique_ptr<vk::DepthBuffer> shadow_map_depth_buffer;
        std::vector<vk::Framebuffer> framebuffers;
        std::unique_ptr<vk::Framebuffer> shadow_map_framebuffer;
        std::unique_ptr<vk::Sampler> shadow_map_sampler;

        // Command pools, semaphores and fences
        std::unique_ptr<vk::CommandPool> command_pool;
        std::vector<vk::CommandBuffer> command_buffers;
        std::vector<vk::Semaphore> image_available_semaphores;
        std::vector<vk::Semaphore> render_finished_semaphores;
        std::vector<vk::Fence> in_flight_fences;

        // Uniform buffers
        std::vector<vk::MappedBuffer> uniform_buffers;
        std::vector<vk::MappedBuffer> shadow_map_uniform_buffers;
        std::vector<vk::MappedBuffer> particle_uniform_buffers;

        // Projection matrices
        glm::mat4 projection_matrix;

        // Mesh data
        std::vector<const Mesh*> shadow_casters_to_render;
        std::vector<InstancedMeshToRender> instanced_non_casters_to_render;
        std::vector<InstancedMeshToRender> instanced_casters_to_render;
        std::vector<ParticlesToRender> particles_to_render;
        std::vector<gfx::vk::MappedBuffer> bounding_boxes_to_render;
        std::vector<gfx::vk::MappedBuffer> instanced_data_buffers;
        std::vector<gfx::vk::MappedBuffer> instanced_shadow_data_buffers;
        std::vector<gfx::vk::MappedBuffer> particle_data_buffers;

        // Diagnostics flags
        // TODO: Move these to context
        bool show_diagnostics;
        bool show_debug_bbs;

        void init_imgui(const Window& window, VkSampleCountFlagBits sample_count);

    };

}