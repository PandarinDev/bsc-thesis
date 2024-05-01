#include "bounding_box.h"
#include "gfx/vk/vertex.h"

#include <cmath>
#include <limits>

namespace inf {

    BoundingBox3D::BoundingBox3D() :
        min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest()) {}

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

    void BoundingBox3D::update(const BoundingBox3D& other) {
        update(other.min);
        update(other.max);
    }

    std::array<glm::vec3, 8> BoundingBox3D::get_points() const {
        return {
            min,                            // Back bottom left
            glm::vec3(min.x, max.y, min.z), // Back top left
            glm::vec3(max.x, max.y, min.z), // Back top right
            glm::vec3(max.x, min.y, min.z), // Back bottom right
            max,                            // Front top right
            glm::vec3(min.x, min.y, max.z), // Front bottom left
            glm::vec3(max.x, min.y, max.z), // Front bottom right
            glm::vec3(min.x, max.y, max.z)  // Front top left
        };
    }

    std::vector<gfx::vk::Vertex> BoundingBox3D::to_vertices(float gap, const glm::vec3& color) const {
        const auto points = get_points();
        std::vector<gfx::vk::Vertex> result;
        
        // Front face
        static const auto front_face_normal = glm::vec3(0.0f, 0.0f, 1.0f);
        const auto front_gap = front_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[5] + front_gap, front_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[6] + front_gap, front_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[4] + front_gap, front_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[5] + front_gap, front_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[4] + front_gap, front_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[7] + front_gap, front_face_normal, color));
        // Back face
        static const auto back_face_normal = glm::vec3(0.0f, 0.0f, -1.0f);
        const auto back_gap = back_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[0] + back_gap, back_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + back_gap, back_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[3] + back_gap, back_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[0] + back_gap, back_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[1] + back_gap, back_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + back_gap, back_face_normal, color));
        // Left face
        static const auto left_face_normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        const auto left_gap = left_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[0] + left_gap, left_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[5] + left_gap, left_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[7] + left_gap, left_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[0] + left_gap, left_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[7] + left_gap, left_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[1] + left_gap, left_face_normal, color));
        // Right face
        static const auto right_face_normal = glm::vec3(1.0f, 0.0f, 0.0f);
        const auto right_gap = right_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[6] + right_gap, right_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[3] + right_gap, right_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + right_gap, right_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[6] + right_gap, right_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + right_gap, right_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[4] + right_gap, right_face_normal, color));
        // Top face
        static const auto top_face_normal = glm::vec3(0.0f, 1.0f, 0.0f);
        const auto top_gap = top_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[7] + top_gap, top_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[4] + top_gap, top_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + top_gap, top_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[7] + top_gap, top_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[2] + top_gap, top_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[1] + top_gap, top_face_normal, color));
        // Bottom face
        static const auto bottom_face_normal = glm::vec3(0.0f, -1.0f, 0.0f);
        const auto bottom_gap = bottom_face_normal * gap;
        result.emplace_back(gfx::vk::Vertex(points[0] + bottom_gap, bottom_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[3] + bottom_gap, bottom_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[6] + bottom_gap, bottom_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[0] + bottom_gap, bottom_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[6] + bottom_gap, bottom_face_normal, color));
        result.emplace_back(gfx::vk::Vertex(points[5] + bottom_gap, bottom_face_normal, color));

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

    OrientedBoundingBox3D BoundingBox3D::to_oriented(const glm::mat4& transformation) const {
        OrientedBoundingBox3D result;
        // These 4 points are enough to determine the basis for the oriented BB
        std::array<glm::vec3, 4> points {
            min,                     // Point of reference
            { max.x, min.y, min.z }, // X axis
            { min.x, max.y, min.z }, // Y axis
            { min.x, min.y, max.z }  // Z axis
        };
        for (std::size_t i = 0; i < points.size(); ++i) {
            points[i] = glm::vec3(transformation * glm::vec4(points[i], 1.0f));
        }
        result.base[0] = points[1] - points[0];
        result.base[1] = points[2] - points[0];
        result.base[2] = points[3] - points[0];
        result.center = points[0] + 0.5f * (result.base[0] + result.base[1] + result.base[2]);
        result.size = glm::vec3(glm::length(result.base[0]), glm::length(result.base[1]), glm::length(result.base[2]));
        for (glm::length_t i = 0; i < 3; ++i) {
            result.base[i] /= result.size[i];
        }
        result.size *= 0.5f;
        return result;
    }

    OrientedBoundingBox3D::OrientedBoundingBox3D(
        const glm::mat3& base,
        const glm::vec3& center,
        const glm::vec3& size) :
        base(base), center(center), size(size) {}

}