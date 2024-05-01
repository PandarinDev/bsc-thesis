#include "road.h"
#include "utils/hash_utils.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using namespace inf;

std::pair<glm::ivec2, DistrictRoad> create_road(RoadDirection direction, const glm::ivec2& position, bool traversable = true) {
    return std::make_pair(position, DistrictRoad(direction, position, traversable, nullptr));
}

TEST_CASE("RoadUtils::get_possible_continuations()") {

    SECTION("returns empty vector if current position is not found") {
        const auto result = RoadUtils::get_possible_continuations({}, glm::ivec2(), RoadDirection::VERTICAL_LEFT);
        REQUIRE(result.empty());
    }

    SECTION("returns correct directions in a crossing where it is possible to turn either left, right or keep straight") {
        const auto result = RoadUtils::get_possible_continuations({
            create_road(RoadDirection::CROSSING_DOWN_RIGHT, glm::ivec2(0, 0)),
            create_road(RoadDirection::CROSSING_DOWN_LEFT, glm::ivec2(-1, 0)),
            create_road(RoadDirection::CROSSING_UP_LEFT, glm::ivec2(-1, -1)),
            create_road(RoadDirection::CROSSING_UP_RIGHT, glm::ivec2(0, -1)),
            create_road(RoadDirection::HORIZONTAL_DOWN, glm::ivec2(1, 0)),
            create_road(RoadDirection::VERTICAL_RIGHT, glm::ivec2(0, -2)),
            create_road(RoadDirection::HORIZONTAL_UP, glm::ivec2(-2, -1))
        }, glm::ivec2(0, 0), RoadDirection::VERTICAL_RIGHT);
        REQUIRE_THAT(result, Catch::Matchers::UnorderedEquals(std::vector<std::vector<glm::ivec2>>{
            { glm::ivec2(1, 0) }, // Right turn
            { glm::ivec2(0, -1), glm::ivec2(0, -2) }, // Going straight
            { glm::ivec2(-1, -1), glm::ivec2(-2, -1) } // Turning left
        }));
    }

    SECTION("does not allow navigation to untraversable roads (at the edges of the districts)") {
        // Only the starting road piece should be traversable
        const auto result = RoadUtils::get_possible_continuations({
            create_road(RoadDirection::CROSSING_DOWN_RIGHT, glm::ivec2(0, 0), true),
            create_road(RoadDirection::CROSSING_DOWN_LEFT, glm::ivec2(-1, 0), false),
            create_road(RoadDirection::CROSSING_UP_LEFT, glm::ivec2(-1, -1), false),
            create_road(RoadDirection::CROSSING_UP_RIGHT, glm::ivec2(0, -1), false),
            create_road(RoadDirection::HORIZONTAL_DOWN, glm::ivec2(1, 0), false),
            create_road(RoadDirection::VERTICAL_RIGHT, glm::ivec2(0, -2), false),
            create_road(RoadDirection::HORIZONTAL_UP, glm::ivec2(-2, -1), false)
        }, glm::ivec2(0, 0), RoadDirection::VERTICAL_RIGHT);
        REQUIRE(result.empty());
    }

}