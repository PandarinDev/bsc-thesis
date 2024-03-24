#include "bounding_box.h"

#include <cmath>
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

    std::vector<glm::ivec3> BoundingBox3D::get_occupied_blocks() const {
        std::vector<glm::ivec3> result;
        const auto min_x = static_cast<int>(std::floorf(min.x));
        const auto max_x = static_cast<int>(std::ceilf(max.x));
        const auto min_y = static_cast<int>(std::floorf(min.y));
        const auto max_y = static_cast<int>(std::ceilf(max.y));
        const auto min_z = static_cast<int>(std::floorf(min.z));
        const auto max_z = static_cast<int>(std::ceilf(max.z));
        for (int x = min_x; x < max_x; ++x) {
            for (int y = min_y; y < max_y; ++y) {
                for (int z = min_z; z < max_z; ++z) {
                    result.emplace_back(x, y, z);
                }
            }
        }
        return result;
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

    BoundingBox3D BoundingBox3D::get_block_to_the_left() const {
        const auto min_x = std::floorf(min.x);
        const auto min_y = std::floorf(min.y);
        const auto max_z = std::ceilf(max.z);
        glm::vec3 left_min(min_x - 1.0f, min_y, max_z - 1.0f);
        glm::vec3 left_max = left_min + glm::vec3(1.0f, 1.0f, 1.0f);
        return BoundingBox3D(left_min, left_max);
    }

    BoundingBox3D BoundingBox3D::get_block_to_the_right() const {
        const auto max_x = std::ceilf(max.x);
        const auto min_y = std::floorf(min.y);
        const auto max_z = std::ceilf(max.z);
        glm::vec3 right_min(max_x, min_y, max_z - 1.0f);
        glm::vec3 right_max = right_min + glm::vec3(1.0f, 1.0f, 1.0f);
        return BoundingBox3D(right_min, right_max);
    }

    BoundingBox3D BoundingBox3D::get_block_above() const {
        const auto min_x = std::floorf(min.x);
        const auto min_y = std::floorf(min.y);
        const auto min_z = std::floorf(min.z);
        glm::vec3 above_min(min_x, min_y, min_z - 1.0f);
        glm::vec3 above_max = above_min + glm::vec3(1.0f, 1.0f, 1.0f);
        return BoundingBox3D(above_min, above_max);
    }

    BoundingBox3D BoundingBox3D::get_block_below() const {
        const auto min_x = std::floorf(min.x);
        const auto min_y = std::floorf(min.y);
        const auto max_z = std::ceilf(max.z);
        glm::vec3 below_min(min_x, min_y, max_z);
        glm::vec3 below_max = below_min + glm::vec3(1.0f, 1.0f, 1.0f);
        return BoundingBox3D(below_min, below_max);
    }

}