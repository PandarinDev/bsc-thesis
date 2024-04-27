#include "gfx/particles.h"
#include "gfx/renderer.h"

namespace inf::gfx {

    std::unique_ptr<Mesh> ParticleMeshes::rain_mesh;

    void ParticleMeshes::initialize(
        const vk::LogicalDevice* device,
        const vk::MemoryAllocator* allocator) {
        static constexpr float rain_half_width = 0.01f;
        static constexpr float rain_half_height = rain_half_width * 2.0f;
        static const std::array<glm::vec3, 6> rain_mesh_vertices {
            glm::vec3(-rain_half_width, -rain_half_height, 0.0f),
            glm::vec3(rain_half_width, -rain_half_height, 0.0f),
            glm::vec3(rain_half_width, rain_half_height, 0.0f),
            glm::vec3(-rain_half_width, -rain_half_height, 0.0f),
            glm::vec3(rain_half_width, rain_half_height, 0.0f),
            glm::vec3(-rain_half_width, rain_half_height, 0.0f)
        };
        static constexpr auto rain_mesh_vertex_bytes = rain_mesh_vertices.size() * sizeof(glm::vec3);
        gfx::vk::MappedBuffer rain_mesh_buffer = gfx::vk::MappedBuffer::create(
            device,
            allocator,
            gfx::vk::BufferType::VERTEX_BUFFER,
            rain_mesh_vertex_bytes);
        rain_mesh_buffer.upload(rain_mesh_vertices.data(), rain_mesh_vertex_bytes);
        rain_mesh = std::make_unique<Mesh>(Mesh(std::move(rain_mesh_buffer), rain_mesh_vertices.size(), glm::mat4(1.0f), BoundingBox3D()));
    }

    void ParticleMeshes::deinitialize() {
        rain_mesh.reset();
    }

    const Mesh& ParticleMeshes::get_rain_mesh() {
        return *rain_mesh;
    }

    ParticleSystem::ParticleSystem(
        const Mesh* mesh,
        RandomGenerator& rng,
        const gfx::Frustum& frustum,
        std::size_t num_max_particles) :
        mesh(mesh), rng(rng), positions(num_max_particles), velocities(num_max_particles) {
        initialize(frustum.split<10>()[0]);
    }

    void ParticleSystem::update(const gfx::Frustum& frustum, float delta_time) {
        const auto frustum_bb = frustum.split<10>()[0].compute_bounding_box();
        std::uniform_real_distribution<float> x_distribution(frustum_bb.min.x, frustum_bb.max.x);
        std::uniform_real_distribution<float> y_distribution(frustum_bb.min.y, frustum_bb.max.y + 0.5f);
        std::uniform_real_distribution<float> z_distribution(frustum_bb.min.z, frustum_bb.max.z);
        std::uniform_real_distribution<float> velocity_distribution(5.0f, 7.0f);
        for (std::size_t i = 0; i < positions.size(); ++i) {
            auto& position = positions[i];
            auto& velocity = velocities[i];
            position.y -= velocity * delta_time;
            // Particles that go below ground level are reused
            if (position.y < 0.0f) {
                position = glm::vec3(
                    x_distribution(rng),
                    y_distribution(rng),
                    z_distribution(rng));
                velocity = velocity_distribution(rng);
            }
        }
    }

    void ParticleSystem::initialize(const gfx::Frustum& frustum) {
        const auto frustum_bb = frustum.compute_bounding_box();
        std::uniform_real_distribution<float> x_distribution(frustum_bb.min.x, frustum_bb.max.x);
        std::uniform_real_distribution<float> y_distribution(frustum_bb.min.y, frustum_bb.max.y + 0.5f);
        std::uniform_real_distribution<float> z_distribution(frustum_bb.min.z, frustum_bb.max.z);
        std::uniform_real_distribution<float> velocity_distribution(5.0f, 7.0f);
        for (std::size_t i = 0; i < positions.size(); ++i) {
            positions[i] = glm::vec3(
                x_distribution(rng),
                y_distribution(rng),
                z_distribution(rng));
            velocities[i] = velocity_distribution(rng);
        }
    }

}