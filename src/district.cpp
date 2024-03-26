#include "district.h"
#include "gfx/renderer.h"

#include <limits>

namespace inf {

    DistrictLot::DistrictLot(
        const glm::ivec2& position,
        const glm::ivec2& dimensions,
        std::optional<wfc::Building>&& building) :
        position(position), dimensions(dimensions), building(std::move(building)) {}

    BoundingBox3D DistrictLot::get_bounding_box(const glm::vec3& district_position) const {
        const auto min = glm::vec3(district_position.x + position.x, 0.0f, district_position.z + position.y);
        const auto max_y = building ? building->get_bounding_box().max.y : 0.0f;
        const auto max = glm::vec3(min.x + dimensions.x, max_y, min.z + dimensions.y);
        return BoundingBox3D(min, max);
    }

    District::District(DistrictType type, const glm::ivec2& grid_position) :
        type(type), grid_position(grid_position), position() {}

    const glm::vec3& District::get_position() const {
        return position;
    }

    void District::set_position(const glm::vec3& position) {
        this->position = position;
        for (auto& lot : lots) {
            if (lot.building) {
                lot.building->set_position(position + glm::vec3(lot.position.x, 0.0f, lot.position.y));
            }
        }
    }

    BoundingBox3D District::compute_bounding_box() const {
        static constexpr auto float_min = std::numeric_limits<float>::lowest();
        static constexpr auto float_max = std::numeric_limits<float>::max();
        if (lots.empty()) {
            return BoundingBox3D(glm::vec3(), glm::vec3());
        }
        BoundingBox3D result(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        // TODO: Considering that buildings do not change this could be cached
        for (const auto& lot : lots) {
            result.update(lot.get_bounding_box(position));
        }
        return result;
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
                positions.emplace_back(x, 0.0f, z);
            }
        }
        renderer.render_instanced(grass_mesh, std::move(positions));

        for (const auto& lot : lots) {
            const auto& building = lot.building;
            if (building) {
                renderer.render(building->get_mesh());
            }
        }
    }

}
