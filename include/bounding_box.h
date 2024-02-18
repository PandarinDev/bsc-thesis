#pragma once

#include <glm/vec3.hpp>

namespace inf {

    struct BoundingBox3D {

        glm::vec3 min;
        glm::vec3 max;

        BoundingBox3D(const glm::vec3& min, const glm::vec3& max);

        glm::vec3 center() const;
        void update(const glm::vec3& position);

    };

}