#pragma once

#include "gfx/mesh.h"
#include "gfx/renderer.h"
#include "utils/hash_utils.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <memory>

namespace inf {

    enum class CellType {
        UNDECIDED,
        FLOWER_SHOP,
        HOUSE
    };

    struct Cell {

        glm::ivec3 coordinates;
        CellType type;
        std::unique_ptr<gfx::Mesh> mesh;

        Cell(const glm::ivec3& coordinates);
        Cell(const glm::ivec3& coordinates, CellType type);

    };

    struct World {

        World();
        World(std::vector<Cell>&& cells);

        Cell& get_or_create_cell(const glm::ivec3& coordinate);

        void render(const gfx::Renderer& renderer);

    private:

        // TODO: Use some spatial data structure instead, this will be slow
        std::vector<Cell> cells;

    };

}