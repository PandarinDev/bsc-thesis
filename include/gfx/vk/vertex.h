#pragma once

#include "bounding_box.h"

#include <glm/vec3.hpp>
#include <glad/vulkan.h>

#include <array>
#include <string>
#include <vector>

namespace inf::gfx::vk {

    // This is an intermediate structure that is "almost a vertex". It's material data needs to be filled out
    // from the chosen material for the given material name of the building that the vertex belongs to.
    struct VertexWithMaterialName {

        glm::vec3 position;
        glm::vec3 normal;
        std::string material_name;

        VertexWithMaterialName(const glm::vec3& position, const glm::vec3& normal, const std::string& material_name);

        static std::vector<VertexWithMaterialName> from_bytes(const std::string& bytes);

    };

    struct Vertex {

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;

        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color);
        Vertex(const VertexWithMaterialName& other, const glm::vec3& color);

        static VkVertexInputBindingDescription get_default_binding_description();
        static std::array<VkVertexInputAttributeDescription, 3> get_default_attribute_descriptions();
        static std::array<VkVertexInputBindingDescription, 2> get_instanced_binding_descriptions();
        static std::array<VkVertexInputAttributeDescription, 5> get_instanced_attribute_descriptions();
        static std::vector<Vertex> from_bytes(const std::string& bytes);
        static BoundingBox3D compute_bounding_box(const std::vector<Vertex>& vertices);

    };

}