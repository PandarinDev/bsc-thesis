#include "gfx/geometry.h"

#include <catch2/catch_test_macros.hpp>

using namespace inf::gfx;

TEST_CASE("Line2D::intersect()") {

    SECTION("returns no intersection for parallel lines with different offsets") {
        Line2D line_a(0.0f, 1.0f);
        Line2D line_b(0.0f, 2.0f);
        REQUIRE(line_a.intersect(line_b) == std::nullopt);
    }

    SECTION("returns intersection at x = 0 for parallel lines with the same offset") {
        Line2D line_a(1.0f, 1.0f);
        Line2D line_b(1.0f, 1.0f);
        REQUIRE(line_a.intersect(line_b) == 0.0f);
    }

    SECTION("returns intersection x coordinate for intersecting lines") {
        Line2D line_a(-1.0f, 5.0f);
        Line2D line_b(0.5f, 2.0f);
        static constexpr float expected_result = 2.0f;
        REQUIRE(line_a.intersect(line_b) == expected_result);
        REQUIRE(line_a.point_at(expected_result) == line_b.point_at(expected_result));
    }

}