#include "bounding_box.h"

#include <catch2/catch_test_macros.hpp>
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
    }

}