#pragma once

#include "common.h"
#include "world.h"
#include "gfx/renderer.h"

#include <deque>

namespace inf {

    struct WorldGenerator {

        WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer);

        World generate_initial();
        void populate_district(District& district);

    private:

        RandomGenerator& random_engine;
        const gfx::Renderer& renderer;

        wfc::Building generate_building();

        void populate_district(District& district, std::deque<DistrictBuilding*>& to_process);

    };

}
