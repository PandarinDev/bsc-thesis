#include "world.h"

namespace inf {

    Cell::Cell(const glm::ivec3& coordinates) : Cell(coordinates, CellType::UNDECIDED) {}

    Cell::Cell(const glm::ivec3& coordinates, CellType type) :
        coordinates(coordinates),
        type(type) {}

    World::World(std::size_t width, std::size_t height) {
        cells.reserve(width * height);
        for (std::size_t x = 0; x < width; ++x) {
            for (std::size_t y = 0; y < height; ++y) {
                cells.emplace_back(glm::ivec3(x, 0, y));
            }
        }
    }

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

    const District* World::get_district_for_cell(const Cell& cell) const {
        const auto& coordinate = glm::vec2(cell.coordinates.x, cell.coordinates.z);
        for (const auto& district : districts) {
            if (district.bounding_box.contains(coordinate)) {
                return &district;
            }
        }
        return nullptr;
    }

    void World::add_district(const District& district) {
        districts.push_back(district);
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