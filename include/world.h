#pragma once

#include "district.h"
#include "gfx/mesh.h"
#include "gfx/renderer.h"
#include "utils/hash_utils.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <memory>

namespace inf {

    struct World {

        std::vector<District> districts;

        void render(const gfx::Renderer& renderer);

    };

}