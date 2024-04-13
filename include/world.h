#pragma once

#include "district.h"
#include "gfx/renderer.h"

#include <unordered_map>

namespace inf {

    struct World {

        World();

        void update_caches();

        bool has_district_at(const glm::ivec2& position) const;
        District& add_district(const glm::ivec2& position, District&& district);
        const std::unordered_map<glm::ivec2, District>& get_districts() const;

        std::size_t get_number_of_districts() const;
        std::size_t get_number_of_buildings() const;
        BoundingBox3D compute_bounding_box() const;

        void update(const gfx::Renderer& renderer);
        void render(gfx::Renderer& renderer);
        bool is_dirty() const;

    private:
    
        std::unordered_map<glm::ivec2, District> districts;
        std::vector<glm::vec3> road_positions;
        std::vector<float> road_rotations;
        std::vector<glm::vec3> crossing_positions;
        std::vector<float> crossing_rotations;
        bool dirty; // Indicates whether caches need to be updated before rendering

        void place_vertical_road(
            const District* left,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& left_roads_at_edges,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& right_roads_at_edges);
        void place_horizontal_road(
            const District* bottom,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& top_roads_at_edges,
            const std::unordered_map<glm::ivec2, const DistrictRoad*>& bottom_roads_at_edges);

    };

}