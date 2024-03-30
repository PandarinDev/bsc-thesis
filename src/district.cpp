#include "district.h"
#include "gfx/renderer.h"

#include <limits>

namespace inf {

    DistrictLot::DistrictLot(
        const glm::ivec2& position,
        const glm::ivec2& dimensions,
        const glm::vec3& bb_color,
        std::optional<wfc::Building>&& building) :
        position(position), dimensions(dimensions), bb_color(bb_color), building(std::move(building)) {}

    BoundingBox3D DistrictLot::get_bounding_box(const glm::vec3& district_position) const {
        const auto min = glm::vec3(district_position.x + position.x, 0.0f, district_position.z + position.y);
        const auto max_y = building ? building->get_bounding_box().max.y : 0.0f;
        const auto max = glm::vec3(min.x + dimensions.x, max_y, min.z + dimensions.y);
        return BoundingBox3D(min, max);
    }

    District::District(
        DistrictType type,
        const glm::ivec2& grid_position,
        const glm::ivec2& dimensions,
        const glm::vec3& bb_color) :
        type(type), grid_position(grid_position), dimensions(dimensions),
        position(), bb_color(bb_color), bounding_box(
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(position.x + dimensions.x, 0.0f, position.z + dimensions.y)) {}

    const glm::vec3& District::get_position() const {
        return position;
    }

    void District::set_position(const glm::vec3& position) {
        this->position = position;
        this->bounding_box = BoundingBox3D(
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(position.x + dimensions.x, 0.0f, position.z + dimensions.y));
        for (auto& lot : lots) {
            if (lot.building) {
                const auto lot_bb = lot.get_bounding_box(position);
                const auto& building_bb = lot.building->get_local_bounding_box();
                const auto half_width_difference = (lot_bb.width() - building_bb.width()) * 0.5f;
                const auto half_depth_difference = (lot_bb.depth() - building_bb.depth()) * 0.5f;
                const auto building_position = glm::vec3(
                    lot_bb.min.x - building_bb.min.x + half_width_difference,
                    0.0f,
                    lot_bb.max.z - building_bb.max.z - half_depth_difference);
                lot.building->set_position(building_position);
            }
        }
    }

    BoundingBox3D District::compute_bounding_box() const {
        return bounding_box;
    }

    const std::vector<DistrictLot>& District::get_lots() const {
        return lots;
    }

    void District::add_lot(DistrictLot&& lot) {
        lots.emplace_back(std::move(lot));
    }

    void District::render(gfx::Renderer& renderer) const {        
        const auto& grass_mesh = wfc::GroundPatterns::get_pattern("grass").mesh;
        const auto bb = compute_bounding_box();
        const auto min_x = std::floorf(bb.min.x);
        const auto max_x = std::ceilf(bb.max.x);
        const auto min_z = std::floorf(bb.min.z);
        const auto max_z = std::ceilf(bb.max.z);
        std::vector<glm::vec3> positions;
        const auto num_ground_objects = static_cast<std::size_t>((max_x - min_x) * (max_z - min_z));
        positions.reserve(num_ground_objects);
        for (float x = min_x; x <= max_x; ++x) {
            for (float z = min_z; z <= max_z; ++z) {
                positions.emplace_back(x, 0.5f, z);
            }
        }
        renderer.render_instanced(grass_mesh, std::move(positions));

        for (const auto& lot : lots) {
            const auto& building = lot.building;
            // renderer.render(lot.get_bounding_box(get_position()), lot.bb_color);
            if (building) {
                renderer.render(building->get_mesh());
                // renderer.render(building->get_bounding_box(), glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }
        renderer.render(compute_bounding_box(), bb_color);
    }

}
