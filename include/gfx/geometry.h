#pragma once

#include <glm/vec3.hpp>

#include <optional>

namespace inf::gfx {

    struct Plane {

        // Parametric equation: dot(p - point, normal) + d = 0

        glm::vec3 point;
        glm::vec3 normal;
        float d;

        Plane(const glm::vec3& point, const glm::vec3& normal, float d);

    };

    struct Ray {

        // Parametric equation: o + t * d = p

        glm::vec3 origin;
        glm::vec3 direction;

        Ray(const glm::vec3& origin, const glm::vec3& direction);

        glm::vec3 point_at(float t) const;

        std::optional<float> intersect(const Plane& plane) const;

    };

}