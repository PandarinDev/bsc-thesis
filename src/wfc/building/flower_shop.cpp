#include "wfc/building/flower_shop.h"
#include "gfx/vk/vertex.h"

#include <vector>

namespace inf::wfc::building {

    FlowerShopRule::FlowerShopRule(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) :
        physical_device(physical_device),
        logical_device(logical_device) {}

    bool FlowerShopRule::matches(const World& world, const Cell& cell) {
        // Any residential building block can be a flower shop
        return true;
    }

    void FlowerShopRule::apply(World&, Cell& cell) {
        cell.type = CellType::FLOWER_SHOP;
        cell.mesh = std::make_unique<gfx::Mesh>(FlowerShopMeshGenerator::generate_mesh(physical_device, logical_device));
    }

    gfx::Mesh FlowerShopMeshGenerator::generate_mesh(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        static glm::vec3 COLOR_RED(1.0f, 0.0f, 0.0f);
        static glm::vec3 NORMAL_FRONT(0.0f, 0.0f, 1.0f);
        static glm::vec3 NORMAL_RIGHT(1.0f, 0.0f, 0.0f);
        static glm::vec3 NORMAL_TOP(0.0f, 1.0f, 0.0f);
        // For now just generate flower shops as 1x1x1 red cubes
        std::vector<gfx::vk::Vertex> vertex_data {
            // Front face
            { { -0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },
            { { 0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },

            { { -0.5f, -0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },
            { { -0.5f, 0.5f, 0.5f }, NORMAL_FRONT, COLOR_RED },

            // Back face
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },

            { { 0.5f, -0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },
            { { 0.5f, 0.5f, -0.5f }, -NORMAL_FRONT, COLOR_RED },

            // Left face
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_RED },
            { { -0.5f, -0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_RED },
            { { -0.5f, 0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_RED },

            { { -0.5f, -0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_RED },
            { { -0.5f, 0.5f, 0.5f }, -NORMAL_RIGHT, COLOR_RED },
            { { -0.5f, 0.5f, -0.5f }, -NORMAL_RIGHT, COLOR_RED },

            // Right face
            { { 0.5f, -0.5f, -0.5f }, NORMAL_RIGHT, COLOR_RED },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_RIGHT, COLOR_RED },
            { { 0.5f, -0.5f, 0.5f }, NORMAL_RIGHT, COLOR_RED },

            { { 0.5f, -0.5f, -0.5f }, NORMAL_RIGHT, COLOR_RED },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_RIGHT, COLOR_RED },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_RIGHT, COLOR_RED },

            // Top face
            { { -0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_RED },
            { { 0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_RED },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_RED },

            { { -0.5f, 0.5f, 0.5f }, NORMAL_TOP, COLOR_RED },
            { { 0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_RED },
            { { -0.5f, 0.5f, -0.5f }, NORMAL_TOP, COLOR_RED },

            // Bottom face
            { { -0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_RED },
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_RED },
            { { 0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_RED },

            { { -0.5f, -0.5f, 0.5f }, -NORMAL_TOP, COLOR_RED },
            { { -0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_RED },
            { { 0.5f, -0.5f, -0.5f }, -NORMAL_TOP, COLOR_RED },
        };

        const auto buffer_size = sizeof(gfx::vk::Vertex) * vertex_data.size();
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device, logical_device, gfx::vk::BufferType::VERTEX_BUFFER, buffer_size);
        vertex_buffer.upload(vertex_data.data(), buffer_size);
        return gfx::Mesh(std::move(vertex_buffer), vertex_data.size(), glm::mat4(1.0f));
    }

}
