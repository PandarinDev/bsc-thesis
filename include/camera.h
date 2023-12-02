#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

namespace inf {

    struct Camera {

        glm::vec3 position;
        glm::vec3 direction;

        Camera() = default;
        Camera(const glm::vec3& position, const glm::vec3& direction);

        glm::mat4 to_view_matrix() const;

        const glm::vec3& get_position() const;
        const glm::vec3& get_direction() const;
        void set_position(const glm::vec3& position);
        void set_direction(const glm::vec3& direction);

    };

}
