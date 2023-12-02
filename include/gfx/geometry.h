#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include <array>
#include <optional>

namespace inf::gfx {

    struct Line2D {

        float slope;
        float offset;

        Line2D(float slope, float offset);

        std::optional<float> intersect(const Line2D& other) const;
        glm::vec2 point_at(float x) const;

    };

    struct Plane {

        glm::vec3 normal;
        float distance;

        Plane(const glm::vec4& vec);
        Plane(const glm::vec3& normal, float distance);

        glm::vec3 get_perpendicular(bool towards_z) const;

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