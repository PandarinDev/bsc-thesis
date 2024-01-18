#include "world.h"
#include "gfx/assets.h"

namespace inf {

    Cell::Cell(const glm::ivec3& coordinates) : Cell(coordinates, CellType::UNDECIDED) {}

    Cell::Cell(const glm::ivec3& coordinates, CellType type) :
        coordinates(coordinates),
        type(type) {}

    World::World() : World({}, {}) {}

    World::World(std::vector<Cell>&& cells, CellCache&& cell_cache) :
        cells(std::move(cells)),
        cell_cache(std::move(cell_cache)) {}

    Cell& World::get_or_create_cell(const glm::ivec3& coordinate) {
        const auto it = cell_cache.find(coordinate);
        if (it != cell_cache.cend()) {
            return *it->second;
        }
        auto& cell = cells.emplace_back(coordinate);
        cell_cache.emplace(coordinate, &cell);
        return cell;
    }

    void World::render(const gfx::Renderer& renderer) {
        for (auto& cell : cells) {
            if (cell.type == CellType::UNDECIDED) {
                continue;
            }
            auto& mesh = gfx::Assets::get_mesh(cell.type);
            mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), glm::vec3(cell.coordinates)));
            renderer.render(mesh);
        }
    }

}