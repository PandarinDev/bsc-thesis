#pragma once

#include "district.h"
#include "gfx/renderer.h"

#include <vector>

namespace inf {

    struct World {

        std::vector<District> districts;

        void render(gfx::Renderer& renderer);

    };

}