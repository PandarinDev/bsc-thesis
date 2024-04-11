#include "gfx/frustum.h"

#include <catch2/catch_test_macros.hpp>

using namespace inf::gfx;

TEST_CASE("Frustum::split()") {

    SECTION("splits the frustum into n subfrustums") {
        static constexpr std::array<glm::vec3, 8> frustum_points {
            // Near plane
            glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f), // Near bottom left
            glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),  // Near bottom right
            glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),  // Near top left
            glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),   // Near top right
            // Far plane
            glm::vec4(-2.0f, -2.0f, 1.0f, 1.0f), // Far bottom left
            glm::vec4(2.0f, -2.0f, 1.0f, 1.0f),  // Far bottom right
            glm::vec4(-2.0f, 2.0f, 1.0f, 1.0f),  // Far top left
            glm::vec4(2.0f, 2.0f, 1.0f, 1.0f),   // Far top right
        };
        const Frustum frustum(frustum_points);
        const auto result = frustum.split<2>();
        REQUIRE(result.size() == 2);
        const auto& first = result[0];
        REQUIRE(first.points[Frustum::NEAR_BOTTOM_LEFT_IDX] == frustum_points[Frustum::NEAR_BOTTOM_LEFT_IDX]);
        REQUIRE(first.points[Frustum::NEAR_BOTTOM_RIGHT_IDX] == frustum_points[Frustum::NEAR_BOTTOM_RIGHT_IDX]);
        REQUIRE(first.points[Frustum::NEAR_TOP_LEFT_IDX] == frustum_points[Frustum::NEAR_TOP_LEFT_IDX]);
        REQUIRE(first.points[Frustum::NEAR_TOP_RIGHT_IDX] == frustum_points[Frustum::NEAR_TOP_RIGHT_IDX]);
        REQUIRE(first.points[Frustum::FAR_BOTTOM_LEFT_IDX] == glm::vec3(-1.5f, -1.5f, 0.5f));
        REQUIRE(first.points[Frustum::FAR_BOTTOM_RIGHT_IDX] == glm::vec3(1.5f, -1.5f, 0.5f));
        REQUIRE(first.points[Frustum::FAR_TOP_LEFT_IDX] == glm::vec3(-1.5f, 1.5f, 0.5f));
        REQUIRE(first.points[Frustum::FAR_TOP_RIGHT_IDX] == glm::vec3(1.5f, 1.5f, 0.5f));

        const auto& second = result[1];
        REQUIRE(second.points[Frustum::NEAR_BOTTOM_LEFT_IDX] == glm::vec3(-1.5f, -1.5f, 0.5f));
        REQUIRE(second.points[Frustum::NEAR_BOTTOM_RIGHT_IDX] == glm::vec3(1.5f, -1.5f, 0.5f));
        REQUIRE(second.points[Frustum::NEAR_TOP_LEFT_IDX] == glm::vec3(-1.5f, 1.5f, 0.5f));
        REQUIRE(second.points[Frustum::NEAR_TOP_RIGHT_IDX] == glm::vec3(1.5f, 1.5f, 0.5f));
        REQUIRE(second.points[Frustum::FAR_BOTTOM_LEFT_IDX] == frustum_points[Frustum::FAR_BOTTOM_LEFT_IDX]);
        REQUIRE(second.points[Frustum::FAR_BOTTOM_RIGHT_IDX] == frustum_points[Frustum::FAR_BOTTOM_RIGHT_IDX]);
        REQUIRE(second.points[Frustum::FAR_TOP_LEFT_IDX] == frustum_points[Frustum::FAR_TOP_LEFT_IDX]);
        REQUIRE(second.points[Frustum::FAR_TOP_RIGHT_IDX] == frustum_points[Frustum::FAR_TOP_RIGHT_IDX]);
    }

}