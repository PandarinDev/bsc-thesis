#pragma once

#include "world.h"
#include "camera.h"
#include "wfc/rule.h"
#include "gfx/geometry.h"

#include <vector>
#include <memory>
#include <random>

namespace inf {

    using WorldRule = wfc::Rule<World, Cell>;

    struct WorldGenerator {

        static World generate_initial(const gfx::Frustum& frustum);

        WorldGenerator(const World* world);

        void collapse(Cell& cell);

    private:

        const World* world;
        std::mt19937 random_engine;
        std::vector<std::unique_ptr<WorldRule>> rules;

    };

}