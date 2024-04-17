#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include <array>
#include <vector>

namespace inf {

    namespace gfx::vk {
        struct Vertex;
    }

    struct OrientedBoundingBox3D;

    struct BoundingBox3D {

        glm::vec3 min;
        glm::vec3 max;

        BoundingBox3D();
        BoundingBox3D(const glm::vec3& min, const glm::vec3& max);

        glm::vec3 center() const;
        void update(const glm::vec3& position);
        void update(const BoundingBox3D& other);
        std::array<glm::vec3, 8> get_points() const;
        std::vector<glm::ivec3> get_occupied_blocks() const;
        std::vector<gfx::vk::Vertex> to_vertices(float gap, const glm::vec3& color) const;

        float width() const;
        float height() const;
        float depth() const;

        BoundingBox3D apply(const glm::mat4& transformation) const;
        BoundingBox3D apply_and_transform_to_ndc(const glm::mat4& transformation) const;
        bool is_inside(const glm::vec3& point) const;
        bool collides(const BoundingBox3D& other) const;

        BoundingBox3D get_block_to_the_left() const;
        BoundingBox3D get_block_to_the_right() const;
        BoundingBox3D get_block_above() const;
        BoundingBox3D get_block_below() const;

        OrientedBoundingBox3D to_oriented(const glm::mat4& transformation) const;

    };

    struct OrientedBoundingBox3D {

        glm::mat3 base;
        glm::vec3 center;
        glm::vec3 size;

        OrientedBoundingBox3D() = default;
        OrientedBoundingBox3D(
            const glm::mat3& base,
            const glm::vec3& center,
            const glm::vec3& size);

    };

}