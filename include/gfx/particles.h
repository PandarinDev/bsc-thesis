#pragma once

#include "common.h"
#include "gfx/mesh.h"
#include "gfx/frustum.h"
#include "gfx/vk/device.h"
#include "gfx/vk/memory_allocator.h"

#include <glm/vec3.hpp>

#include <memory>
#include <vector>

namespace inf::gfx {

    struct ParticleMeshes {

        ParticleMeshes() = delete;

        static void initialize(
            const vk::LogicalDevice* device,
            const vk::MemoryAllocator* allocator);
        static void deinitialize();

        static const Mesh& get_rain_mesh();

    private:

        static std::unique_ptr<Mesh> rain_mesh;

    };

    struct ParticleSystem {

        const Mesh* mesh;
        RandomGenerator& rng;
        std::vector<glm::vec3> positions;
        std::vector<float> velocities;

        ParticleSystem(
            const Mesh* mesh,
            RandomGenerator& rng,
            const gfx::Frustum& frustum,
            std::size_t num_max_particles);

        void update(const gfx::Frustum& frustum, float delta_time);

    private:

        void initialize(const gfx::Frustum& frustum);

    };

}