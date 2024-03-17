#include "gfx/vk/vertex.h"

namespace inf::gfx::vk {

    Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color) :
        position(position),
        normal(normal),
        color(color) {}

    std::array<VkVertexInputBindingDescription, 2> Vertex::get_binding_descriptions() {
        std::array<VkVertexInputBindingDescription, 2> binding_descriptions;

        auto& per_vertex_binding_description = binding_descriptions[0];
        per_vertex_binding_description.binding = 0;
        per_vertex_binding_description.stride = sizeof(Vertex);
        per_vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        auto& per_instance_binding_description = binding_descriptions[1];
        per_instance_binding_description.binding = 1;
        per_instance_binding_description.stride = sizeof(glm::vec3);
        per_instance_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return binding_descriptions;
    }

    std::array<VkVertexInputAttributeDescription, 4> Vertex::get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions;

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

        // Position (vec3) - per instance
        auto& instance_position = attribute_descriptions[3];
        instance_position.binding = 1;
        instance_position.location = 3;
        instance_position.format = VK_FORMAT_R32G32B32_SFLOAT;
        instance_position.offset = 0;

        return attribute_descriptions;
    }

    std::vector<Vertex> Vertex::from_bytes(const std::string& bytes_str) {
        static constexpr auto floats_per_vertex = 9;
        const auto num_vertices = bytes_str.size() / sizeof(float) / floats_per_vertex;
        const float* data = reinterpret_cast<const float*>(bytes_str.data());
        std::vector<Vertex> result;
        for (std::size_t i = 0; i < num_vertices; ++i) {
            result.emplace_back(
                glm::vec3(data[0], data[1], data[2]),
                glm::vec3(data[3], data[4], data[5]),
                glm::vec3(data[6], data[7], data[8])
            );
            data += floats_per_vertex;
        }
        return result;
    }

}