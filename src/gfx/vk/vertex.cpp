#include "gfx/vk/vertex.h"

namespace inf::gfx::vk {

    Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color) :
        position(position),
        normal(normal),
        color(color) {}

    VkVertexInputBindingDescription Vertex::get_binding_description() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions;

        // Position (vec3)
        auto& position_attribute = attribute_descriptions[0];
        position_attribute.binding = 0;
        position_attribute.location = 0;
        position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        position_attribute.offset = offsetof(Vertex, position);

        // Normal (vec3)
        auto& normal_attribute = attribute_descriptions[1];
        normal_attribute.binding = 0;
        normal_attribute.location = 1;
        normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        normal_attribute.offset = offsetof(Vertex, normal);

        // Color (vec3)
        auto& color_attribute = attribute_descriptions[2];
        color_attribute.binding = 0;
        color_attribute.location = 2;
        color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        color_attribute.offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }

}