#include "utils/hash_utils.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Hash implementation for glm::ivec3") {

    SECTION("Returns the same hash for identical vectors") {
        std::hash<glm::ivec3> hasher{};
        REQUIRE(hasher(glm::ivec3(1, 2, 3)) == hasher(glm::ivec3(1, 2, 3)));
    }

    SECTION("Returns different hash for different vectors") {
        std::hash<glm::ivec3> hasher{};
        REQUIRE(hasher(glm::ivec3(1, 2, 3)) != hasher(glm::ivec3(4, 5, 6)));
    }

}
