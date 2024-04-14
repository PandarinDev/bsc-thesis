#include "utils/math_utils.h"

#include <catch2/catch_test_macros.hpp>

using namespace inf::utils;

TEST_CASE("MathUtils::lerp()") {

    SECTION("Interpolates between 2 values") {
        {
            std::vector<float> values = { 1.0f, 2.0f };
            REQUIRE(MathUtils::lerp(values, 0.0f) == 1.0f);
            REQUIRE(MathUtils::lerp(values, 0.5f) == 1.5f);
            REQUIRE(MathUtils::lerp(values, 1.0f) == 2.0f);
        }

        {
            std::vector<glm::vec3> values = {
                glm::vec3(0.0f, 1.0f, 2.0f),
                glm::vec3(1.0f, 2.0f, 3.0f)
            };
            REQUIRE(MathUtils::lerp(values, 0.0f) == glm::vec3(0.0f, 1.0f, 2.0f));
            REQUIRE(MathUtils::lerp(values, 0.5f) == glm::vec3(0.5f, 1.5f, 2.5f));
            REQUIRE(MathUtils::lerp(values, 1.0f) == glm::vec3(1.0f, 2.0f, 3.0f));
        }
    }

    SECTION("Interpolates between 3 values") {
        {
            std::vector<float> values = { 1.0f, 2.0f, 4.0f };
            REQUIRE(MathUtils::lerp(values, 0.0f) == 1.0f);
            REQUIRE(MathUtils::lerp(values, 0.1f) == 1.2f);
            REQUIRE(MathUtils::lerp(values, 0.5f) == 2.0f);
            REQUIRE(MathUtils::lerp(values, 0.6f) == 2.4f);
            REQUIRE(MathUtils::lerp(values, 1.0f) == 4.0f);
        }
    }

}