#include "gfx/mesh.h"
#include "gfx/vk/vertex.h"

#include <array>

namespace inf::gfx {

    Mesh::Mesh(vk::MappedBuffer&& buffer, std::size_t num_vertices, const glm::mat4& model_matrix) :
        buffer(std::move(buffer)),
        num_vertices(num_vertices),
        model_matrix(model_matrix) {}
    
    std::size_t Mesh::get_number_of_vertices() const {
        return num_vertices;
    }

    const vk::MappedBuffer& Mesh::get_buffer() const {
        return buffer;
    }

    const glm::mat4& Mesh::get_model_matrix() const {
        return model_matrix;
    }

    void Mesh::set_model_matrix(const glm::mat4& model_matrix) {
        this->model_matrix = model_matrix;
    }

    Mesh Cube::build(
        const vk::PhysicalDevice& physical_device,
        const vk::LogicalDevice* logical_device,
        float size,
        const glm::vec3& color) {
        float hsize = size * 0.5f;
        std::array<vk::Vertex, 36> vertices = {
            // Front face
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, hsize), color },

            // Back face
            vk::Vertex{ glm::vec3(hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, -hsize), color },

            // Top face
            vk::Vertex{ glm::vec3(-hsize, hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, -hsize), color },

            // Bottom face
            vk::Vertex{ glm::vec3(hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, -hsize), color },

            // Right face
            vk::Vertex{ glm::vec3(hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(hsize, hsize, hsize), color },

            // Left face
            vk::Vertex{ glm::vec3(-hsize, -hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, -hsize), color },
            vk::Vertex{ glm::vec3(-hsize, -hsize, hsize), color },
            vk::Vertex{ glm::vec3(-hsize, hsize, hsize), color },
        };
        const auto size_bytes = sizeof(vk::Vertex) * vertices.size();
        auto buffer = vk::MappedBuffer::create(
            physical_device,
            logical_device,
            vk::BufferType::VERTEX_BUFFER,
            size_bytes);
        buffer.upload(vertices.data(), size_bytes);

        return Mesh(std::move(buffer), vertices.size(), glm::mat4(1.0f));
    }

}