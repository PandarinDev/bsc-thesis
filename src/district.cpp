#include "district.h"
#include "gfx/renderer.h"

#include <limits>

namespace inf {

    District::District(DistrictType type) : type(type) {}

    const std::vector<wfc::Building>& District::get_buildings() const {
        return buildings;
    }

    void District::add_building(wfc::Building&& building) {
        auto blocks = building.get_bounding_box().get_occupied_blocks();
        occupied_blocks.insert(blocks.begin(), blocks.end());
        buildings.emplace_back(std::move(building));
    }

    bool District::can_place(const BoundingBox3D& bb) const {
        auto blocks = bb.get_occupied_blocks();
        for (const auto& block : blocks) {
            if (occupied_blocks.find(block) != occupied_blocks.cend()) {
                return false;
            }
        }
        return true;
    }

    BoundingBox3D District::compute_bounding_box() const {
        static constexpr auto float_min = std::numeric_limits<float>::lowest();
        static constexpr auto float_max = std::numeric_limits<float>::max();
        BoundingBox3D result(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        for (const auto& building : buildings) {
            const auto building_bb = building.get_bounding_box();
            result.update(building_bb.min);
            result.update(building_bb.max);
        }
        return result;
    }

    void District::update(const gfx::Renderer& renderer) {
        // Remove buildings that are no longer visible
        std::vector<wfc::Building> buildings_to_keep;
        std::unordered_set<glm::ivec3> new_occupied_blocks;
        buildings_to_keep.reserve(buildings.size());
        for (auto& building : buildings) {
            const auto bb = building.get_bounding_box();
            if (renderer.is_in_view(bb)) {
                const auto occupied = bb.get_occupied_blocks();
                new_occupied_blocks.insert(occupied.cbegin(), occupied.cend());
                buildings_to_keep.emplace_back(std::move(building));
            }
        }
        buildings = std::move(buildings_to_keep);
        occupied_blocks = std::move(new_occupied_blocks);
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

        for (const auto& building : buildings) {
            renderer.render(building.get_mesh());
        }
    }

}
