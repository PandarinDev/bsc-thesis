#include "gfx/vk/vertex.h"

namespace inf::gfx::vk {

    VertexWithMaterialName::VertexWithMaterialName(const glm::vec3& position, const glm::vec3& normal, const std::string& material_name) :
        position(position), normal(normal), material_name(material_name) {}

    std::vector<VertexWithMaterialName> VertexWithMaterialName::from_bytes(const std::string& bytes) {
        const char* data = bytes.data();
        const char* end_ptr = bytes.data() + bytes.size();
        std::vector<VertexWithMaterialName> result;
        while (data < end_ptr) {
            const auto get_float = [data](std::size_t offset) {
                return *reinterpret_cast<const float*>(data + offset * sizeof(float));
            };
            std::uint8_t material_name_length = *reinterpret_cast<const std::uint8_t*>(data + 6 * sizeof(float));
            result.emplace_back(
                glm::vec3(get_float(0), get_float(1), get_float(2)),
                glm::vec3(get_float(3), get_float(4), get_float(5)),
                std::string(data + 6 * sizeof(float) + 1, material_name_length)
            );
            data += 6 * sizeof(float) + 1 + material_name_length;
        }
        return result;
    }

    Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color) :
        position(position),
        normal(normal),
        color(color) {}

    Vertex::Vertex(const VertexWithMaterialName& other, const glm::vec3& color) :
        position(other.position), normal(other.normal), color(color) {}

    VkVertexInputBindingDescription Vertex::get_default_binding_description() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::get_default_attribute_descriptions() {
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

    std::array<VkVertexInputBindingDescription, 2> Vertex::get_instanced_binding_descriptions() {
        std::array<VkVertexInputBindingDescription, 2> binding_descriptions;

        auto& per_vertex_binding_description = binding_descriptions[0];
        per_vertex_binding_description.binding = 0;
        per_vertex_binding_description.stride = sizeof(Vertex);
        per_vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        auto& per_instance_binding_description = binding_descriptions[1];
        per_instance_binding_description.binding = 1;
        per_instance_binding_description.stride = sizeof(glm::vec3) + sizeof(float);
        per_instance_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return binding_descriptions;
    }

    std::array<VkVertexInputAttributeDescription, 5> Vertex::get_instanced_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 5> attribute_descriptions;

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

        // Instance position
        auto& instance_position = attribute_descriptions[3];
        instance_position.binding = 1;
        instance_position.location = 3;
        instance_position.format = VK_FORMAT_R32G32B32_SFLOAT;
        instance_position.offset = 0;

        // Instance rotation
        auto& instance_rotation = attribute_descriptions[4];
        instance_rotation.binding = 1;
        instance_rotation.location = 4;
        instance_rotation.format = VK_FORMAT_R32_SFLOAT;
        instance_rotation.offset = sizeof(glm::vec3);

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

    BoundingBox3D Vertex::compute_bounding_box(const std::vector<Vertex>& vertices) {
        BoundingBox3D result;
        for (const auto& vertex : vertices) {
            result.update(vertex.position);
        }
        return result;
    }

}