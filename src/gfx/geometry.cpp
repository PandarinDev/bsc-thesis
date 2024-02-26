#include "gfx/geometry.h"

#include <glm/glm.hpp>

namespace inf::gfx {

    Plane::Plane(const glm::vec3& point, const glm::vec3& normal, float d) :
        point(point), normal(normal), d(d) {}

    Ray::Ray(const glm::vec3& origin, const glm::vec3& direction) :
        origin(origin), direction(direction) {}

    glm::vec3 Ray::point_at(float t) const {
        return origin + direction * t;
    }

    std::optional<float> Ray::intersect(const Plane& plane) const {
        static constexpr float threshold = 1e-6f;
        const auto denominator = glm::dot(direction, plane.normal);
        // If the ray and the plane are (almost) parallel we say that there is no intersection
        if (denominator <= threshold) {
            return std::nullopt;
        }
        const auto t = glm::dot(plane.point - origin, plane.normal) / denominator;
        // If the intersection is backward compared to ray direction, throw it away
        return t >= 0.0f ? t : 0.0f;
    }

}