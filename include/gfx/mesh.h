#pragma once

#include "gfx/vk/buffer.h"
#include "gfx/vk/device.h"
#include "bounding_box.h"

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

namespace inf::gfx {

    struct Mesh {

        Mesh(
            vk::MappedBuffer&& buffer,
            std::size_t num_vertices,
            const glm::mat4& model_matrix,
            const BoundingBox3D& bounding_box);

        const vk::MappedBuffer& get_buffer() const;
        std::size_t get_number_of_vertices() const;
        const glm::mat4& get_model_matrix() const;
        const BoundingBox3D& get_bounding_box_in_model_space() const;

        void set_model_matrix(const glm::mat4& model_matrix);

    private:

        vk::MappedBuffer buffer;
        std::size_t num_vertices;
        glm::mat4 model_matrix;
        BoundingBox3D bounding_box;

    };

}