#include "gfx/frustum.h"
#include "gfx/renderer.h"

namespace inf::gfx {

    Frustum::Frustum(const glm::mat4& matrix) : points(extract_points(matrix)) {}

    Frustum::Frustum(const std::array<glm::vec3, 8>& points) : points(points) {}

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
        const auto x_near = points[NEAR_BOTTOM_RIGHT_IDX].x;
        const auto y_near = points[NEAR_BOTTOM_RIGHT_IDX].y;
        const auto z_near = Renderer::NEAR_PLANE;
        const auto z_far = Renderer::FAR_PLANE;

        // Using separating axis theorem (SAT) to check if we can find one
        // separating axis of the 26 possibilities that when used the projected
        // coordinates of the frustum and the OBB calculates whether there are
        // any intersections. Resource: https://bruop.github.io/improved_frustum_culling/
        std::vector<glm::vec3> test_axes;
        // Base of the OBB
        for (glm::length_t i = 0; i < 3; ++i) {
            test_axes.emplace_back(obb.base[i]);
        }
        // Unique normals of the frustum
        for (const auto& normal : get_unique_normals()) {
            test_axes.emplace_back(normal);
        }
        // Base crossed with up, right and points of the near plane
        for (glm::length_t i = 0; i < 3; ++i) {
            test_axes.emplace_back(glm::cross(obb.base[i], glm::vec3(0.0f, 1.0f, 0.0f)));
            test_axes.emplace_back(glm::cross(obb.base[i], glm::vec3(1.0f, 0.0f, 0.0f)));
            // We only need the points of the near plane
            for (std::size_t j = NEAR_BOTTOM_LEFT_IDX; j <= NEAR_TOP_RIGHT_IDX; ++j) {
                test_axes.emplace_back(glm::cross(obb.base[i], points[j]));
            }
        }

        // Compare against the tests axes
        for (const auto& axis : test_axes) {
            float mox = std::abs(axis.x);
            float moy = std::abs(axis.y);
            float moz = axis.z;
            float moc = glm::dot(axis, obb.center);

            float radius = 0.0f;
            for (glm::length_t i = 0; i < 3; ++i) {
                radius += std::abs(glm::dot(axis, obb.base[i])) * obb.size[i];
            }
            float min = moc - radius;
            float max = moc + radius;
            float p = x_near * mox + y_near * moy;
            float tau_0 = z_near * moz - p;
            float tau_1 = z_near * moz + p;

            if (tau_0 < 0.0f) {
                tau_0 *= z_far / z_near;
            }

            if (tau_1 > 0.0f) {
                tau_1 *= z_far / z_near;
            }

            if (min > tau_1 || max < tau_0) {
                return false;
            }
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

    std::array<glm::vec3, 5> Frustum::get_unique_normals() const {
        const auto x_near = points[NEAR_BOTTOM_RIGHT_IDX].x;
        const auto y_near = points[NEAR_BOTTOM_RIGHT_IDX].y;
        const auto z_near = Renderer::NEAR_PLANE;
        return {
            glm::vec3{ z_near, 0.0f, -x_near },
            glm::vec3{ -z_near, 0.0f, -x_near },
            glm::vec3{ 0.0f, -z_near, -y_near },
            glm::vec3{ 0.0f, z_near, -y_near },
            glm::vec3{ 0.0f, 0.0f, 1.0f }
        };
    }

}