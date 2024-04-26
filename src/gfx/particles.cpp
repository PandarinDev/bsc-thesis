#include "gfx/particles.h"
#include "gfx/renderer.h"

namespace inf::gfx {

    ParticleSystem::ParticleSystem(
        Mesh&& mesh,
        RandomGenerator& rng,
        const gfx::Frustum& frustum,
        std::size_t num_max_particles) :
        mesh(std::move(mesh)), rng(rng), positions(num_max_particles), velocities(num_max_particles) {
        initialize(frustum.split<10>()[0]);
    }

    void ParticleSystem::update(const gfx::Frustum& frustum, float delta_time) {
        const auto frustum_bb = frustum.compute_bounding_box();
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