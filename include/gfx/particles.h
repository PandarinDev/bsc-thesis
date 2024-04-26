#pragma once

#include "common.h"
#include "gfx/mesh.h"
#include "gfx/frustum.h"

#include <glm/vec3.hpp>

#include <vector>

namespace inf::gfx {

    struct ParticleSystem {

        Mesh mesh;
        RandomGenerator& rng;
        std::vector<glm::vec3> positions;
        std::vector<float> velocities;

        ParticleSystem(
            Mesh&& mesh,
            RandomGenerator& rng,
            const gfx::Frustum& frustum,
            std::size_t num_max_particles);

        void update(const gfx::Frustum& frustum, float delta_time);

    private:

        void initialize(const gfx::Frustum& frustum);

    };

}