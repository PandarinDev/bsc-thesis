#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include <array>

namespace inf::gfx {

    struct Plane {

        glm::vec3 normal;
        float distance;

        Plane(const glm::vec4& vec);
        Plane(const glm::vec3& normal, float distance);

    };

    struct Frustum {

        Frustum(const glm::mat4& view_projection_matrix);

        bool is_inside(const glm::vec3& block_coordinate) const;

    private:

        static std::array<Plane, 6> extract_planes(const glm::mat4& view_projection_matrix);

        glm::mat4 view_projection_matrix;
        std::array<Plane, 6> planes;

    };

}