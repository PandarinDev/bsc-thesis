#include "gfx/frustum.h"

namespace inf::gfx {

    Plane::Plane(const glm::vec4& vec) :
        normal(glm::vec3(vec)),
        distance(vec.w) {}

    Plane::Plane(const glm::vec3& normal, float distance) :
        normal(normal), distance(distance) {}

    Frustum::Frustum(const glm::mat4& view_projection_matrix) :
        view_projection_matrix(view_projection_matrix),
        planes(extract_planes(view_projection_matrix)) {}

    bool Frustum::is_inside(const glm::vec3& block_coordinate) const {
        const auto transformed_coordinate = glm::vec3(view_projection_matrix * glm::vec4(block_coordinate, 1.0f));
        for (const auto& plane : planes) {
            // TODO This currently only check if the middle of the block is visible,
            // which will result in culling blocks where only the corner would be visible.
            if (glm::dot(plane.normal, transformed_coordinate) <= 0.0f) {
                return false;
            }
        }
        return true;
    }

    std::array<Plane, 6> Frustum::extract_planes(const glm::mat4& view_projection_matrix) {
        return {
            Plane(glm::normalize(view_projection_matrix[3] + view_projection_matrix[0])), // Left plane
            Plane(glm::normalize(view_projection_matrix[3] - view_projection_matrix[0])), // Right plane
            Plane(glm::normalize(view_projection_matrix[3] - view_projection_matrix[1])), // Top plane
            Plane(glm::normalize(view_projection_matrix[3] + view_projection_matrix[1])), // Bottom plane
            Plane(glm::normalize(view_projection_matrix[3] - view_projection_matrix[2])), // Near plane
            Plane(glm::normalize(view_projection_matrix[3] + view_projection_matrix[2])), // Far plane
        };
    }

}