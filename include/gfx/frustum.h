#pragma once

#include "bounding_box.h"

#include <glm/matrix.hpp>

#include <array>

namespace inf::gfx {

    struct Frustum {

        static constexpr auto
            NEAR_BOTTOM_LEFT_IDX = 0,
            NEAR_BOTTOM_RIGHT_IDX = 1,
            NEAR_TOP_LEFT_IDX = 2,
            NEAR_TOP_RIGHT_IDX = 3,
            FAR_BOTTOM_LEFT_IDX = 4,
            FAR_BOTTOM_RIGHT_IDX = 5,
            FAR_TOP_LEFT_IDX = 6,
            FAR_TOP_RIGHT_IDX = 7;
        std::array<glm::vec3, 8> points;

        Frustum() = default;
        Frustum(const glm::mat4& matrix);
        Frustum(const std::array<glm::vec3, 8>& points);

        glm::vec3 center() const;
        BoundingBox3D compute_bounding_box() const;

        template<std::size_t split_count>
        std::array<Frustum, split_count> split() const {
            static constexpr float step = 1.0f / static_cast<float>(split_count);
            std::array<std::array<glm::vec3, 8>, split_count> result;
            for (std::size_t i = 0; i < split_count; ++i) {
                const auto t = i * step;
                const auto next_t = (i + 1) * step;
                // Near plane
                result[i][NEAR_BOTTOM_LEFT_IDX] = glm::mix(points[NEAR_BOTTOM_LEFT_IDX], points[FAR_BOTTOM_LEFT_IDX], t);
                result[i][NEAR_BOTTOM_RIGHT_IDX] = glm::mix(points[NEAR_BOTTOM_RIGHT_IDX], points[FAR_BOTTOM_RIGHT_IDX], t);
                result[i][NEAR_TOP_LEFT_IDX] = glm::mix(points[NEAR_TOP_LEFT_IDX], points[FAR_TOP_LEFT_IDX], t);
                result[i][NEAR_TOP_RIGHT_IDX] = glm::mix(points[NEAR_TOP_RIGHT_IDX], points[FAR_TOP_RIGHT_IDX], t);
                // Far plane
                result[i][FAR_BOTTOM_LEFT_IDX] = glm::mix(points[NEAR_BOTTOM_LEFT_IDX], points[FAR_BOTTOM_LEFT_IDX], next_t);
                result[i][FAR_BOTTOM_RIGHT_IDX] = glm::mix(points[NEAR_BOTTOM_RIGHT_IDX], points[FAR_BOTTOM_RIGHT_IDX], next_t);
                result[i][FAR_TOP_LEFT_IDX] = glm::mix(points[NEAR_TOP_LEFT_IDX], points[FAR_TOP_LEFT_IDX], next_t);
                result[i][FAR_TOP_RIGHT_IDX] = glm::mix(points[NEAR_TOP_RIGHT_IDX], points[FAR_TOP_RIGHT_IDX], next_t);
            }

            std::array<Frustum, split_count> frustums;
            for (std::size_t i = 0; i < split_count; ++i) {
                frustums[i] = Frustum(result[i]);
            }
            return frustums;
        }

    private:

        static std::array<glm::vec3, 8> extract_points(const glm::mat4& matrix);

    };

}