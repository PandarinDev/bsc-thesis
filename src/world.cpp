#include "world.h"

#include <cmath>

namespace inf {

    World::World(gfx::Mesh&& cube_mesh) : cube_mesh(std::move(cube_mesh)) {
        const auto tan_value = std::tanf(gfx::Renderer::FOVY / 2.0f);
        // We slice up the frustum to block sized slices and decide how many blocks are visible in each row
        for (float depth = std::floorf(gfx::Renderer::NEAR_PLANE); depth <= 10.0f; ++depth) {
            const auto blocks_in_row = std::ceilf(tan_value * depth * 2.0f);
            // Add the offsets for the row
            const auto x_offset_start = -blocks_in_row * 0.5f;
            for (std::uint32_t i = 0; i < blocks_in_row; ++i) {
                block_offsets.emplace_back(glm::vec2(x_offset_start + i, depth));
            }
        }
    }

    void World::render(const gfx::Renderer& renderer) {
        for (const auto& offset : block_offsets) {
            cube_mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), glm::vec3(offset.x, 0.0f, -offset.y)));
            renderer.render(cube_mesh);
        }
    }

}