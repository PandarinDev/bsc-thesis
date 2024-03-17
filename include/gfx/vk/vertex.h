#pragma once

#include <glm/vec3.hpp>
#include <glad/vulkan.h>

#include <array>
#include <string>
#include <vector>

namespace inf::gfx::vk {

    struct Vertex {

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;

        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color);

        static std::array<VkVertexInputBindingDescription, 2> get_binding_descriptions();
        static std::array<VkVertexInputAttributeDescription, 4> get_attribute_descriptions();
        static std::vector<Vertex> from_bytes(const std::string& bytes);

    };

}