#include "gfx/frustum.h"

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

}