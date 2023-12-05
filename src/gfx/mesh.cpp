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

}