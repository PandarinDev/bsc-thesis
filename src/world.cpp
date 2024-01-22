#include "world.h"

namespace inf {

    Cell::Cell(const glm::ivec3& coordinates) : Cell(coordinates, CellType::UNDECIDED) {}

    Cell::Cell(const glm::ivec3& coordinates, CellType type) :
        coordinates(coordinates),
        type(type) {}

    World::World() : World(std::vector<Cell>{}) {}

    World::World(std::vector<Cell>&& cells) :
        cells(std::move(cells)) {}

    Cell& World::get_or_create_cell(const glm::ivec3& coordinate) {
        for (auto& cell : cells) {
            if (cell.coordinates == coordinate) {
                return cell;
            }
        }
        return cells.emplace_back(coordinate);
    }

    void World::render(const gfx::Renderer& renderer) {
        for (auto& cell : cells) {
            if (cell.mesh) {
                cell.mesh->set_model_matrix(glm::translate(glm::mat4(1.0f), glm::vec3(cell.coordinates)));
                renderer.render(*cell.mesh);
            }
        }
    }

}