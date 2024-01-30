#include "gfx/geometry.h"

namespace inf::gfx {

    void Cube::add_to(
        std::vector<vk::Vertex>& vertices,
        const glm::vec3& from,
        const glm::vec3& to,
        const glm::vec3& color) {
        static const glm::vec3 NORMAL_FORWARD(0.0f, 0.0f, 1.0f);
        static const glm::vec3 NORMAL_BACKWARD(0.0f, 0.0f, -1.0f);
        static const glm::vec3 NORMAL_UP(0.0f, 1.0f, 0.0f);
        static const glm::vec3 NORMAL_DOWN(0.0f, -1.0f, 0.0f);
        static const glm::vec3 NORMAL_LEFT(-1.0f, 0.0f, 0.0f);
        static const glm::vec3 NORMAL_RIGHT(1.0f, 0.0f, 0.0f);
        // Front face
        vertices.emplace_back(
            glm::vec3(from.x, from.y, from.z),
            NORMAL_FORWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, from.z),
            NORMAL_FORWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, from.z),
            NORMAL_FORWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, from.z),
            NORMAL_FORWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, from.z),
            NORMAL_FORWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, from.z),
            NORMAL_FORWARD,
            color
        );
        
        // Back face
        vertices.emplace_back(
            glm::vec3(from.x, from.y, to.z),
            NORMAL_BACKWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, to.z),
            NORMAL_BACKWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, to.z),
            NORMAL_BACKWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, to.z),
            NORMAL_BACKWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, to.z),
            NORMAL_BACKWARD,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, to.z),
            NORMAL_BACKWARD,
            color
        );

        // Top face
        vertices.emplace_back(
            glm::vec3(from.x, to.y, from.z),
            NORMAL_UP,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, from.z),
            NORMAL_UP,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, to.z),
            NORMAL_UP,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, from.z),
            NORMAL_UP,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, to.z),
            NORMAL_UP,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, to.z),
            NORMAL_UP,
            color
        );

        // Bottom face
        vertices.emplace_back(
            glm::vec3(from.x, from.y, from.z),
            NORMAL_DOWN,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, to.z),
            NORMAL_DOWN,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, from.z),
            NORMAL_DOWN,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, from.z),
            NORMAL_DOWN,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, to.z),
            NORMAL_DOWN,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, to.z),
            NORMAL_DOWN,
            color
        );

        // Left face
        vertices.emplace_back(
            glm::vec3(from.x, from.y, to.z),
            NORMAL_LEFT,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, from.z),
            NORMAL_LEFT,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, from.z),
            NORMAL_LEFT,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, from.y, to.z),
            NORMAL_LEFT,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, from.z),
            NORMAL_LEFT,
            color
        );
        vertices.emplace_back(
            glm::vec3(from.x, to.y, to.z),
            NORMAL_LEFT,
            color
        );

        // Right face
        vertices.emplace_back(
            glm::vec3(to.x, from.y, to.z),
            NORMAL_RIGHT,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, from.z),
            NORMAL_RIGHT,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, from.z),
            NORMAL_RIGHT,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, from.y, to.z),
            NORMAL_RIGHT,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, to.z),
            NORMAL_RIGHT,
            color
        );
        vertices.emplace_back(
            glm::vec3(to.x, to.y, from.z),
            NORMAL_RIGHT,
            color
        );
    }

}