#include "wfc/building/house.h"
#include "gfx/vk/vertex.h"

#include <vector>

namespace inf::wfc::building {

    HouseRule::HouseRule(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) :
        physical_device(physical_device),
        logical_device(logical_device) {}

    bool HouseRule::matches(const World& world, const Cell& cell) {
        // TODO: Should decide based on whether the cell is in a residential district.
        return true;
    }

    void HouseRule::apply(World&, Cell& cell) {
        cell.type = CellType::HOUSE;
        cell.mesh = std::make_unique<gfx::Mesh>(HouseMeshGenerator::generate_mesh(physical_device, logical_device));
    }

    gfx::Mesh HouseMeshGenerator::generate_mesh(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        static glm::vec3 COLOR_BLUE(0.0f, 0.0f, 1.0f);
        static glm::vec3 NORMAL_FRONT(0.0f, 0.0f, 1.0f);
        static glm::vec3 NORMAL_RIGHT(1.0f, 0.0f, 0.0f);
        static glm::vec3 NORMAL_TOP(0.0f, 1.0f, 0.0f);
        // For now just generate houses as 1x1x1 blue cubes
        std::vector<gfx::vk::Vertex> vertex_data {
            // Front face
            { { -0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },
            { { 0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },

            { { -0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },
            { { -0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_BLUE },

            // Back face
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },

            { { 0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },
            { { 0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_BLUE },

            // Left face
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_BLUE },
            { { -0.5f, -0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_BLUE },
            { { -0.5f, 0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_BLUE },

            { { -0.5f, -0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_BLUE },
            { { -0.5f, 0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_BLUE },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_BLUE },

            // Right face
            { { 0.5f, -0.5f, -0.5f }, NORMAL_RIGHT, COLOR_BLUE },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_RIGHT, COLOR_BLUE },
            { { 0.5f, -0.5f, 0.5f }, NORMAL_RIGHT, COLOR_BLUE },

            { { 0.5f, -0.5f, -0.5f }, NORMAL_RIGHT, COLOR_BLUE },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_RIGHT, COLOR_BLUE },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_RIGHT, COLOR_BLUE },

            // Top face
            { { -0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_BLUE },

            { { -0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_BLUE },
            { { -0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_BLUE },

            // Bottom face
            { { -0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_BLUE },

            { { -0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_BLUE },
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_BLUE },
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_BLUE },
        };

        const auto buffer_size = sizeof(gfx::vk::Vertex) * vertex_data.size();
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device, logical_device, gfx::vk::BufferType::VERTEX_BUFFER, buffer_size);
        vertex_buffer.upload(vertex_data.data(), buffer_size);
        return gfx::Mesh(std::move(vertex_buffer), vertex_data.size(), glm::mat4(1.0f));
    }

}
