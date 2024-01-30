#include "wfc/building/flower_shop.h"
#include "gfx/geometry.h"

#include <vector>

namespace inf::wfc::building {

    FlowerShopRule::FlowerShopRule(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) :
        physical_device(physical_device),
        logical_device(logical_device) {}

    bool FlowerShopRule::matches(const World& world, const Cell& cell) {
        const auto maybe_district = world.get_district_for_cell(cell);
        return maybe_district && maybe_district->type == DistrictType::RESIDENTAL;
    }

    void FlowerShopRule::apply(World&, Cell& cell) {
        cell.type = CellType::FLOWER_SHOP;
        cell.mesh = std::make_unique<gfx::Mesh>(FlowerShopMeshGenerator::generate_mesh(physical_device, logical_device));
    }

    gfx::Mesh FlowerShopMeshGenerator::generate_mesh(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        // For now just generate flower shops as 1x1x1 red cubes
        std::vector<gfx::vk::Vertex> vertex_data;
        gfx::Cube::add_to(vertex_data, glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        const auto buffer_size = sizeof(gfx::vk::Vertex) * vertex_data.size();
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device, logical_device, gfx::vk::BufferType::VERTEX_BUFFER, buffer_size);
        vertex_buffer.upload(vertex_data.data(), buffer_size);
        return gfx::Mesh(std::move(vertex_buffer), vertex_data.size(), glm::mat4(1.0f));
    }

}
