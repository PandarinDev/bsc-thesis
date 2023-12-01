#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace inf {

    Camera::Camera(const glm::vec3& position, const glm::vec3& direction) :
        position(position),
        direction(direction) {}

    glm::mat4 Camera::to_view_matrix() const {
        static const glm::vec3 up(0.0f, 1.0f, 0.0f);
        return glm::lookAt(position, position + direction, up);
    }

}