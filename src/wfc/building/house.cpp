#include "wfc/building/house.h"
#include "wfc/rule.h"
#include "gfx/geometry.h"

#include <vector>
#include <cstdint>

namespace inf::wfc::building {

    static constexpr auto MIN_WIDTH = 5;
    static constexpr auto MAX_WIDTH = 10;
    static constexpr auto MIN_DEPTH = 2;
    static constexpr auto MAX_DEPTH = 4;
    static constexpr auto HEIGHT = 2;

    enum class HouseCellType {
        EMPTY,
        WALL,
        DOOR,
        WINDOW,
        CORNER,
        ROOF,
        ROOF_CORNER
    };

    struct HouseContext {};

    struct HouseCell {

        glm::vec3 coordinates;    
        HouseCellType type;
        bool is_corner;
        bool is_edge;
        bool is_roof;

        HouseCell(const glm::vec3& coordinates) :
            coordinates(coordinates),
            type(HouseCellType::EMPTY),
            is_corner(false),
            is_edge(false),
            is_roof(false) {}

    };

    using HouseRule = Rule<HouseContext, HouseCell>;

    const auto CellTypeApplicator = [](const HouseCellType type) {
        return [type](HouseContext&, HouseCell& cell) {
            cell.type = type;
        };
    };

    static const std::vector<HouseRule> HOUSE_RULES = {
        // Wall rule
        {
            [](const HouseContext&, const HouseCell& cell) {
                return cell.is_edge && !cell.is_roof;
            },
            CellTypeApplicator(HouseCellType::WALL)
        }
    };

    Building HouseGenerator::generate(
        RandomGenerator& rng,
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        std::uniform_int_distribution<int> width_distribution(MIN_WIDTH, MAX_WIDTH);
        std::uniform_int_distribution<int> depth_distribution(MIN_DEPTH, MAX_DEPTH);
        std::uniform_int_distribution<int> color_distribution(0, 255);
        int width = width_distribution(rng);
        int depth = depth_distribution(rng);
        glm::vec3 color(
            color_distribution(rng) / 255.0f,
            color_distribution(rng) / 255.0f,
            color_distribution(rng) / 255.0f);

        HouseContext context;
        std::vector<HouseCell> cells;
        for (float y = 0; y < HEIGHT; ++y) {
            for (float x = 0; x < width; ++x) {
                for (float z = 0; z < depth; ++z) {
                    auto& cell = cells.emplace_back(glm::vec3(x, y, z));
                    cell.is_corner =
                        (x == 0 || x == width - 1) &&
                        (y == 0 || y == HEIGHT - 1) &&
                        (z == 0 || z == depth - 1);
                    cell.is_edge =
                        x == 0 || x == width - 1 ||
                        z == 0 || z == depth - 1;
                    cell.is_roof = y == HEIGHT - 1;
                }
            }
        }
        wfc_collapse(rng, context, cells, HOUSE_RULES);

        // Convert the collapsed cells to meshes
        // TODO: Use a lookup map instead
        std::vector<gfx::vk::Vertex> vertices;
        for (const auto& cell : cells) {
            if (cell.type == HouseCellType::WALL) {
                glm::vec3 from = cell.coordinates - glm::vec3(0.5f, 0.5f, 0.5f);
                glm::vec3 to = cell.coordinates + glm::vec3(0.5f, 0.5f, 0.5f);
                gfx::Cube::add_to(vertices, from, to, color);
            }
        }

        // Upload vertex data to the GPU and return the generated building
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device,
            logical_device,
            gfx::vk::BufferType::VERTEX_BUFFER,
            vertices.size() * sizeof(gfx::vk::Vertex));
        vertex_buffer.upload(vertex_buffer);
        gfx::Mesh mesh(std::move(vertex_buffer), vertices.size(), glm::mat4(1.0f));
        return Building(BuildingType::HOUSE, Direction::SOUTH, std::move(mesh));
    }

}
