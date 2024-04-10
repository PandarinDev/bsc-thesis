#include "world.h"

namespace inf {

    bool World::has_district_at(const glm::ivec2& position) const {
        return districts.find(position) != districts.cend();
    }

    std::size_t World::get_number_of_districts() const {
        return districts.size();
    }

    std::size_t World::get_number_of_buildings() const {
        std::size_t accumulator = 0;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            // This uses the assumption that there is exactly one building per lot
            accumulator += district.get_lots().size();
        }
        return accumulator;
    }

    BoundingBox3D World::compute_bounding_box() const {
        BoundingBox3D result;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            result.update(district.compute_bounding_box());
        }
        return result;
    }

    void World::update(const gfx::Renderer& renderer) {
        std::vector<glm::ivec2> keys_to_remove;
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            const auto district_bb = district.compute_bounding_box();
            if (!renderer.is_in_view(district_bb)) {
                keys_to_remove.emplace_back(entry.first);
            }
        }
        for (const auto& key : keys_to_remove) {
            districts.erase(key);
        }
    }

    void World::render(gfx::Renderer& renderer) {
        for (const auto& entry : districts) {
            const auto& district = entry.second;
            district.render(renderer);
        }
    }

}
