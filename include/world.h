#pragma once

#include "gfx/mesh.h"
#include "gfx/renderer.h"
#include "utils/hash_utils.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

namespace inf {

    enum class CellType {
        UNDECIDED,
        HOUSE
    };

    struct Cell {

        glm::ivec3 coordinates;
        CellType type;

        Cell(const glm::ivec3& coordinates);
        Cell(const glm::ivec3& coordinates, CellType type);

    };

    using CellCache = std::unordered_map<glm::ivec3, Cell*>;

    struct World {

        World();
        World(std::vector<Cell>&& cells, CellCache&& cell_cache);

        Cell& get_or_create_cell(const glm::ivec3& coordinate);

        void render(const gfx::Renderer& renderer);

    private:

        std::vector<Cell> cells;
        CellCache cell_cache;

    };

}