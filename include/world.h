#pragma once

#include "gfx/mesh.h"
#include "gfx/renderer.h"

#include <glm/vec2.hpp>

#include <vector>
#include <memory>

namespace inf {

    struct World {

        World(gfx::Mesh&& cube_mesh);

        void render(const gfx::Renderer& renderer);

    private:

        gfx::Mesh cube_mesh;
        std::vector<glm::vec2> block_offsets;

    };

}