#include "bounding_box.h"

#include <array>
#include <limits>

namespace inf {

    BoundingBox3D::BoundingBox3D(const glm::vec3& min, const glm::vec3& max) :
        min(min), max(max) {}

    glm::vec3 BoundingBox3D::center() const {
        return (min + max) * 0.5f;
    }

    void BoundingBox3D::update(const glm::vec3& position) {
        if (position.x < min.x) min.x = position.x;
        if (position.y < min.y) min.y = position.y;
        if (position.z < min.z) min.z = position.z;
        if (position.x > max.x) max.x = position.x;
        if (position.y > max.y) max.y = position.y;
        if (position.z > max.z) max.z = position.z;
    }

    BoundingBox3D BoundingBox3D::apply(const glm::mat4& transformation) const {
        std::array<glm::vec3, 8> points = {
            min,                            // Front bottom left
            glm::vec3(min.x, max.y, min.z), // Front top left
            glm::vec3(max.x, max.y, min.z), // Front top right
            glm::vec3(max.x, min.y, min.z), // Front bottom right
            max,                            // Back top right
            glm::vec3(min.x, min.y, max.z), // Back bottom left
            glm::vec3(max.x, min.y, max.z), // Back bottom right
            glm::vec3(min.x, max.y, max.z)  // Back top left
        };
        static constexpr auto min_float = std::numeric_limits<float>::min();
        static constexpr auto max_float = std::numeric_limits<float>::max();
        glm::vec3 new_min(max_float, max_float, max_float);
        glm::vec3 new_max(min_float, min_float, min_float);
        for (const auto& point : points) {
            const auto transformed = glm::vec3(transformation * glm::vec4(point, 1.0f));
            if (transformed.x < new_min.x) new_min.x = transformed.x;
            if (transformed.x > new_max.x) new_max.x = transformed.x;
            if (transformed.y < new_min.y) new_min.y = transformed.y;
            if (transformed.y > new_max.y) new_max.y = transformed.y;
            if (transformed.z < new_min.z) new_min.z = transformed.z;
            if (transformed.z > new_max.z) new_max.z = transformed.z;
        }

        return BoundingBox3D(new_min, new_max);
    }

}