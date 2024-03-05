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

    std::array<glm::vec3, 8> BoundingBox3D::get_points() const {
        return {
            min,                            // Front bottom left
            glm::vec3(min.x, max.y, min.z), // Front top left
            glm::vec3(max.x, max.y, min.z), // Front top right
            glm::vec3(max.x, min.y, min.z), // Front bottom right
            max,                            // Back top right
            glm::vec3(min.x, min.y, max.z), // Back bottom left
            glm::vec3(max.x, min.y, max.z), // Back bottom right
            glm::vec3(min.x, max.y, max.z)  // Back top left
        };
    }

    float BoundingBox3D::width() const {
        return max.x - min.x;
    }

    float BoundingBox3D::height() const {
        return max.y - min.y;
    }

    float BoundingBox3D::depth() const {
        return max.z - min.z;
    }

    BoundingBox3D BoundingBox3D::apply(const glm::mat4& transformation) const {
        static constexpr auto min_float = std::numeric_limits<float>::lowest();
        static constexpr auto max_float = std::numeric_limits<float>::max();
        
        BoundingBox3D result(
            glm::vec3(max_float, max_float, max_float),
            glm::vec3(min_float, min_float, min_float)
        );
        const auto points = get_points();
        for (const auto& point : points) {
            auto transformed = transformation * glm::vec4(point, 1.0f);
            result.update(glm::vec3(transformed));
        }
        return result;
    }

    BoundingBox3D BoundingBox3D::apply_and_transform_to_ndc(const glm::mat4& transformation) const {
        static constexpr auto min_float = std::numeric_limits<float>::lowest();
        static constexpr auto max_float = std::numeric_limits<float>::max();
        static const auto is_clipped = [](const glm::vec4& point) {
            return
                point.x < -point.w || point.x > point.w ||
                point.y < -point.w || point.y > point.w ||
                point.z < -point.w || point.z > point.w;
        };

        BoundingBox3D result(
            glm::vec3(max_float, max_float, max_float),
            glm::vec3(min_float, min_float, min_float)
        );
        const auto points = get_points();
        for (const auto& point : points) {
            auto transformed = transformation * glm::vec4(point, 1.0f);
            // Apply clipping manually
            if (is_clipped(transformed)) {
                continue;
            }
            transformed /= transformed.w;
            result.update(glm::vec3(transformed));
        }
        return result;
    }

    bool BoundingBox3D::is_inside(const glm::vec3& point) const {
        return point.x >= min.x && point.y >= min.y && point.z >= min.z &&
            point.x <= max.x && point.y <= max.y && point.z <= max.z;
    }

    bool BoundingBox3D::collides(const BoundingBox3D& other) const {
        static const auto overlaps = [](float min1, float min2, float max1, float max2) {
            return max1 >= min2 && max2 >= min1;
        };
        return overlaps(min.x, other.min.x, max.x, other.max.x) &&
            overlaps(min.y, other.min.y, max.y, other.max.y) &&
            overlaps(min.z, other.min.z, max.z, other.max.z);
    }

}