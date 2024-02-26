#pragma once

#include "common.h"
#include "world.h"
#include "gfx/renderer.h"

namespace inf {

    struct WorldGenerator {

        static World generate_initial(const gfx::Renderer& renderer);

        WorldGenerator(World* world);

        void generate(const gfx::Renderer& renderer);

    private:

        World* world;
        RandomGenerator random_engine;

    };

}
