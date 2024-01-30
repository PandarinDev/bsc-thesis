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
        const auto maybe_district = world.get_district_for_cell(cell);
        return maybe_district && maybe_district->type == DistrictType::RESIDENTAL;
    }

    void HouseRule::apply(World&, Cell& cell) {
        cell.type = CellType::HOUSE;
        cell.mesh = std::make_unique<gfx::Mesh>(HouseMeshGenerator::generate_mesh(physical_device, logical_device));
    }

    gfx::Mesh HouseMeshGenerator::generate_mesh(
        const gfx::vk::PhysicalDevice& physical_device,
        const gfx::vk::LogicalDevice* logical_device) {
        // For now just generate houses as 1x1x1 blue cubes
        std::vector<gfx::vk::Vertex> vertex_data;
        gfx::Cube::add_to(vertex_data, glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f));

        const auto buffer_size = sizeof(gfx::vk::Vertex) * vertex_data.size();
        auto vertex_buffer = gfx::vk::MappedBuffer::create(
            physical_device, logical_device, gfx::vk::BufferType::VERTEX_BUFFER, buffer_size);
        vertex_buffer.upload(vertex_data.data(), buffer_size);
        return gfx::Mesh(std::move(vertex_buffer), vertex_data.size(), glm::mat4(1.0f));
    }

}
