#include "bounding_box.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace inf;

TEST_CASE("BoundingBox3D::center()") {

    SECTION("calculates center") {
        const BoundingBox3D bounding_box(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(10.0f, 10.0f, 10.0f));
        REQUIRE(bounding_box.center() == glm::vec3(5.0f, 5.0f, 5.0f));
    }

}

TEST_CASE("BoundingBox3D::update()") {

    SECTION("updates bounding box with new point") {
        BoundingBox3D bounding_box(
            glm::vec3(3.0f, 3.0f, 3.0f),
            glm::vec3(6.0f, 6.0f, 6.0f));
        bounding_box.update(glm::vec3(0.0f, 1.0f, 8.0f));
        REQUIRE(bounding_box.min == glm::vec3(0.0f, 1.0f, 3.0f));
        REQUIRE(bounding_box.max == glm::vec3(6.0f, 6.0f, 8.0f));
    }

    SECTION("updates bounding box with another bounding box's points") {
        BoundingBox3D first(
            glm::vec3(1.0f, 2.0f, 3.0f),
            glm::vec3(4.0f, 5.0f, 6.0f));
        // The second bounding box is ill-formed, since the max value is smaller than the min, but it does not matter
        const BoundingBox3D second(
            glm::vec3(0.0f, 3.0f, 8.0f),
            glm::vec3(2.0f, 6.0f, 4.0f));
        first.update(second);
        REQUIRE(first.min == glm::vec3(0.0f, 2.0f, 3.0f));
        REQUIRE(first.max == glm::vec3(4.0f, 6.0f, 8.0f));
    }

}

TEST_CASE("BoundingBox3D::get_points()") {

    SECTION("returns the 8 points of the bounding box") {
        const BoundingBox3D bounding_box(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(10.0f, 10.0f, 10.0f));
        const auto result_array = bounding_box.get_points();
        // Converting to vector to use unordered equals matcher
        const std::vector<glm::vec3> result(result_array.cbegin(), result_array.cend());
        REQUIRE(result.size() == 8);
        REQUIRE_THAT(result, Catch::Matchers::UnorderedEquals(std::vector<glm::vec3>{
            glm::vec3(0.0f, 0.0f, 0.0f),   // Near bottom left
            glm::vec3(10.0f, 0.0f, 0.0f),  // Near bottom right
            glm::vec3(10.0f, 10.0f, 0.0f), // Near up right
            glm::vec3(0.0f, 10.0f, 0.0f),  // Near up left
            glm::vec3(0.0f, 0.0f, 10.0f),   // Far bottom left
            glm::vec3(10.0f, 0.0f, 10.0f),  // Far bottom right
            glm::vec3(10.0f, 10.0f, 10.0f), // Far up right
            glm::vec3(0.0f, 10.0f, 10.0f),  // Far up left
        }));
    }

}

TEST_CASE("BoundingBox3D::width()") {

    SECTION("calculates width") {
        const BoundingBox3D bounding_box(
            glm::vec3(5.0f, 0.0f, 0.0f),
            glm::vec3(10.0f, 0.0f, 0.0f));
        REQUIRE(bounding_box.width() == 5.0f);
    }

}

TEST_CASE("BoundingBox3D::height()") {

    SECTION("calculates height") {
        const BoundingBox3D bounding_box(
            glm::vec3(0.0f, 3.0f, 0.0f),
            glm::vec3(0.0f, 6.0f, 0.0f));
        REQUIRE(bounding_box.height() == 3.0f);
    }

}

TEST_CASE("BoundingBox3D::depth()") {

    SECTION("calculates depth") {
        const BoundingBox3D bounding_box(
            glm::vec3(0.0f, 0.0f, -2.0f),
            glm::vec3(0.0f, 0.0f, 4.0f));
        REQUIRE(bounding_box.depth() == 6.0f);
    }

}

TEST_CASE("BoundingBox3D::apply()") {

    SECTION("applies transformation and picks new min and max points") {
        const BoundingBox3D bounding_box(
            glm::vec3(-10.0f, -10.0f, -10.0f),
            glm::vec3(10.0f, 10.0f, 10.0f));
        const auto identity_bb = bounding_box.apply(glm::mat4(1.0f));
        REQUIRE(identity_bb.min == bounding_box.min);
        REQUIRE(identity_bb.max == bounding_box.max);

        const auto scaled_bb = bounding_box.apply(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f)));
        REQUIRE(scaled_bb.min == bounding_box.min * 2.0f);
        REQUIRE(scaled_bb.max == bounding_box.max * 2.0f);

        const auto flipped_bb = bounding_box.apply(glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, -1.0f)));
        REQUIRE(flipped_bb.min == bounding_box.min);
        REQUIRE(flipped_bb.max == bounding_box.max);

        const auto translated_bb = bounding_box.apply(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)));
        REQUIRE(translated_bb.min == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(translated_bb.max == glm::vec3(20.0f, 20.0f, 20.0f));
    }

}

TEST_CASE("BoundingBox3D::to_oriented()") {

    SECTION("Converts axis-aligned bounding box to oriented bounding box") {
        const BoundingBox3D aabb(
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f));
        const auto forty_five_degree_rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        const OrientedBoundingBox3D obb = aabb.to_oriented(forty_five_degree_rotation);
        // Check that the base is orthonormal
        for (glm::length_t i = 0; i < 3; ++i) {
            REQUIRE(glm::length(obb.base[i]) == 1.0f);
            for (glm::length_t j = 0; j < 3; ++j) {
                if (i != j) {
                    REQUIRE(glm::dot(obb.base[i], obb.base[j]) == 0.0f);
                }
            }
        }
        // Verify that the center did not change due to the rotation
        REQUIRE(obb.center == glm::vec3(0.0f, 0.0f, 0.0f));
        // Size should remain 1 in each direction (within a small tolerance)
        for (glm::length_t i = 0; i < 3; ++i) {
            REQUIRE_THAT(obb.size[i], Catch::Matchers::WithinAbs(1.0f, 1e-7));
        }
    }

}