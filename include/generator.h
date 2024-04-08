#pragma once

#include "common.h"
#include "world.h"
#include "gfx/renderer.h"

#include <deque>

namespace inf {

    struct WorldGenerator {

        WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer);

        World generate_initial();
        void populate_world(World& world);

    private:

        RandomGenerator& random_engine;
        const gfx::Renderer& renderer;

        District generate_district(const glm::ivec2& grid_position);
        wfc::Building generate_building(const wfc::BuildingPattern& pattern, int max_width, int max_depth);

    };

}
