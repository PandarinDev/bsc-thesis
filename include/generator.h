#pragma once

#include "common.h"
#include "world.h"
#include "gfx/renderer.h"

#include <glm/vec2.hpp>

#include <unordered_map>

namespace inf {

    struct WorldGenerator {

        WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer);

        World generate_initial(const Timer& timer);
        void populate_world(World& world);

    private:

        RandomGenerator& random_engine;
        const gfx::Renderer& renderer;

        District generate_district(const glm::ivec2& grid_position);
        wfc::Building generate_building(const wfc::BuildingPattern& pattern, int max_width, int max_depth);
        bool has_road_direction(const std::unordered_map<glm::ivec2, DistrictRoad>& roads, const glm::ivec2& position, RoadDirection direction);
        void set_road_directions(std::unordered_map<glm::ivec2, DistrictRoad>& roads);

    };

}
