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
        std::vector<wfc::Building> buildings_to_keep;
        buildings_to_keep.reserve(buildings.size());
        for (auto& building : buildings) {
            if (renderer.is_in_view(building.get_bounding_box())) {
                buildings_to_keep.emplace_back(std::move(building));
            }
        }
        buildings = std::move(buildings_to_keep);
    }

    void District::render(gfx::Renderer& renderer) const {
        // TODO: Ground meshes are not dynamically generated with WFC, so instead of assigning
        // a mesh to each there should be a mesh pool that the instances choose from.
        for (auto& ground : grounds) {
            ground.mesh.set_model_matrix(glm::translate(glm::mat4(1.0f), ground.position));
            renderer.render(ground.mesh);
        }
        for (const auto& building : buildings) {
            renderer.render(building.get_mesh());
        }
    }

}
