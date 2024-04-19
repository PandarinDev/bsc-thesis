#include "gfx/frustum.h"
#include "gfx/renderer.h"

namespace inf::gfx {

    Frustum::Frustum(const glm::mat4& matrix) :
        points(extract_points(matrix)), up(extract_up(matrix)), right(extract_right(matrix)) {}

    Frustum::Frustum(
        const std::array<glm::vec3, 8>& points,
        const glm::vec3& up,
        const glm::vec3& right) : points(points), up(up), right(right) {}

    glm::vec3 Frustum::center() const {
        glm::vec3 result;
        for (const auto& point : points) {
            result += point;
        }
        result /= static_cast<float>(points.size());
        return result;
    }

    BoundingBox3D Frustum::compute_bounding_box() const {
        BoundingBox3D result;
        for (const auto& point : points) {
            result.update(point);
        }
        return result;
    }

    bool Frustum::is_inside(const OrientedBoundingBox3D& obb) const {
        // Using separating axis theorem (SAT) to check if we can find one
        // separating axis of the 26 possibilities that when used the projected
        // coordinates of the frustum and the OBB calculates whether there are
        // any intersections. Resource: https://bruop.github.io/improved_frustum_culling/
        const auto is_separating_axis = [this, &obb](const glm::vec3& axis) {
            // Project OBB points to axis
            float obb_min = std::numeric_limits<float>::max();
            float obb_max = std::numeric_limits<float>::lowest();
            const auto obb_points = obb.get_points();
            for (const auto& point : obb_points) {
                const auto projected = glm::dot(point, axis);
                obb_min = std::min(obb_min, projected);
                obb_max = std::max(obb_max, projected);
            }

            // Project frustum to axis
            float frustum_min = std::numeric_limits<float>::max();
            float frustum_max = std::numeric_limits<float>::lowest();
            for (const auto& point : points) {
                const auto projected = glm::dot(point, axis);
                frustum_min = std::min(frustum_min, projected);
                frustum_max = std::max(frustum_max, projected);
            }

            // Check if ranges are separate
            return obb_min > frustum_max || frustum_min > obb_max;
        };

        // Start by checking frustum normals
        const auto normals = get_unique_normals();
        for (const auto& normal : normals) {
            if (is_separating_axis(normal)) {
                return false;
            }
        }

        // Check OBB axes
        for (glm::length_t i = 0; i < 3; ++i) {
            if (is_separating_axis(obb.base[i])) {
                return false;
            }
        }

        // Check cross products
        for (glm::length_t i = 0; i < 3; ++i) {
            if (obb.base[i] != up && is_separating_axis(glm::normalize(glm::cross(obb.base[i], up)))) {
                return false;
            }

            if (obb.base[i] != right && is_separating_axis(glm::normalize(glm::cross(obb.base[i], right)))) {
                return false;
            }

            // TODO: Add cross product of edges of frustum
        }

        return true;
    }

    std::array<glm::vec3, 8> Frustum::extract_points(const glm::mat4& matrix) {
        static constexpr std::array<glm::vec4, 8> ndc_points = {
            // Near plane
            glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f), // Near bottom left
            glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),  // Near bottom right
            glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),  // Near top left
            glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),   // Near top right
            // Far plane
            glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), // Far bottom left
            glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),  // Far bottom right
            glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),  // Far top left
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),   // Far top right
        };

        const auto inverse = glm::inverse(matrix);
        std::array<glm::vec3, 8> result;
        for (std::size_t i = 0; i < result.size(); ++i) {
            const auto point = inverse * ndc_points[i];
            const auto result_i = point / point.w;
            result[i] = glm::vec3(result_i);
        }
        return result;
    }

    glm::vec3 Frustum::extract_up(const glm::mat4& matrix) {
        return matrix[1];
    }

    glm::vec3 Frustum::extract_right(const glm::mat4& matrix) {
        return matrix[0];
    }

    std::array<glm::vec3, 5> Frustum::get_unique_normals() const {
        const auto forward_top_left = points[FAR_TOP_LEFT_IDX] - points[NEAR_TOP_LEFT_IDX];
        const auto forward_top_right = points[FAR_TOP_RIGHT_IDX] - points[NEAR_TOP_RIGHT_IDX];
        const auto forward_bottom_left = points[FAR_BOTTOM_LEFT_IDX] - points[NEAR_BOTTOM_LEFT_IDX];
        const auto far_up = points[FAR_TOP_LEFT_IDX] - points[FAR_BOTTOM_LEFT_IDX];
        const auto far_right = points[FAR_BOTTOM_RIGHT_IDX] - points[FAR_BOTTOM_LEFT_IDX];
        const auto far_normal = glm::normalize(glm::cross(far_right, far_up));
        const auto left_normal = glm::normalize(glm::cross(forward_top_left, far_up));
        const auto right_normal = glm::normalize(glm::cross(far_up, forward_top_right));
        const auto top_normal = glm::normalize(glm::cross(forward_top_left, far_right));
        const auto bottom_normal = glm::normalize(glm::cross(far_right, forward_bottom_left));

        return {
            far_normal,
            left_normal,
            right_normal,
            top_normal,
            bottom_normal
        };
    }

}