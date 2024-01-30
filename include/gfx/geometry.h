#pragma once

#include "gfx/vk/vertex.h"

#include <glm/vec3.hpp>

#include <vector>

namespace inf::gfx {

    struct Cube {

        Cube() = delete;

        static void add_to(
            std::vector<vk::Vertex>& vertices,
            const glm::vec3& from,
            const glm::vec3& to,
            const glm::vec3& color);

    };

}