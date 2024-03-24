#include "district.h"
#include "gfx/renderer.h"

#include <limits>

namespace inf {

    DistrictBuilding::DistrictBuilding(const glm::ivec2& position, wfc::Building&& building) :
        position(position), building(std::move(building)), top(nullptr), right(nullptr), bottom(nullptr), left(nullptr) {}

    District::District(DistrictType type) : type(type) {}

    DistrictBuildings& District::get_buildings() {
        return buildings;
    }

    const DistrictBuildings& District::get_buildings() const {
        return buildings;
    }

    DistrictBuilding* District::add_building(const glm::ivec2& position, wfc::Building&& building) {
        auto blocks = building.get_bounding_box().get_occupied_blocks();
        auto [it, inserted] = buildings.emplace(std::piecewise_construct,
            std::forward_as_tuple(position),
            std::forward_as_tuple(position, std::move(building)));
        return inserted ? &it->second : nullptr;
    }

    BoundingBox3D District::compute_bounding_box() const {
        static constexpr auto float_min = std::numeric_limits<float>::lowest();
        static constexpr auto float_max = std::numeric_limits<float>::max();
        if (buildings.empty()) {
            return BoundingBox3D(glm::vec3(), glm::vec3());
        }
        BoundingBox3D result(
            glm::vec3(float_max, float_max, float_max),
            glm::vec3(float_min, float_min, float_min));
        for (const auto& entry : buildings) {
            const auto& building = entry.second.building;
            const auto building_bb = building.get_bounding_box();
            result.update(building_bb.min);
            result.update(building_bb.max);
        }
        return result;
    }

    void District::update(const gfx::Renderer& renderer) {
        // Remove buildings that are no longer visible
        std::vector<glm::ivec2> keys_to_remove;
        for (const auto& entry : buildings) {
            const auto& building = entry.second.building;
            const auto bb = building.get_bounding_box();
            // Stretch the bounding box in each direction to avoid pops (especially for shadows)
            const BoundingBox3D stretched_bb(
                bb.min + glm::vec3(-2.0f, 0.0f, -2.0f),
                bb.max + glm::vec3(2.0f, 0.0f, 2.0f));
            if (!renderer.is_in_view(stretched_bb)) {
                const auto& position = entry.first;
                keys_to_remove.emplace_back(position);
                std::vector<std::pair<glm::ivec2, std::function<void(DistrictBuilding&)>>> to_update {
                    { { 1, 0 }, [](DistrictBuilding& right_neighbor) { right_neighbor.left = nullptr; } },
                    { { -1, 0 }, [](DistrictBuilding& left_neighbor) { left_neighbor.right = nullptr; } },
                    { { 0, 1 }, [](DistrictBuilding& top_neighbor) { top_neighbor.bottom = nullptr; } },
                    { { 0, -1 }, [](DistrictBuilding& bottom_neighbor) { bottom_neighbor.top = nullptr; } }
                };
                for (const auto& update : to_update) {
                    const auto update_position = position + update.first;
                    if (auto it = buildings.find(update_position); it != buildings.cend()) {
                        update.second(it->second);
                    }
                }
            }
        }

        for (const auto& key : keys_to_remove) {
            buildings.erase(key);
        }
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

        for (const auto& entry : buildings) {
            const auto& building = entry.second.building;
            renderer.render(building.get_mesh());
        }
    }

}
