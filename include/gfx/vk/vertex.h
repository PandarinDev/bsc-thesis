#pragma once

#include <glm/vec3.hpp>
#include <glad/vulkan.h>

#include <array>

namespace inf::gfx::vk {

    struct Vertex {

        glm::vec3 position;
        glm::vec3 color;

        Vertex(const glm::vec3& position, const glm::vec3& color);

        static VkVertexInputBindingDescription get_binding_description();
        static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();

    };

}