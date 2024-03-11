#pragma once

#include "common.h"
#include "world.h"
#include "gfx/renderer.h"

namespace inf {

    struct WorldGenerator {

        WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer);

        World generate_initial();
        void populate_district(District& district);
        void populate_district_edges(District& district);

    private:

        RandomGenerator& random_engine;
        const gfx::Renderer& renderer;

        wfc::Building generate_building();

    };

}
