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

    };

}
