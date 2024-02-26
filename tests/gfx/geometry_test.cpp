#include "gfx/geometry.h"

#include <catch2/catch_test_macros.hpp>

using namespace inf::gfx;

TEST_CASE("Ray::point_at()") {

    SECTION("returns the 3D coordinate at point t") {
        const Ray ray(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        REQUIRE(ray.point_at(1.0f) == glm::vec3(3.0f, 3.0f, 4.0f));
        REQUIRE(ray.point_at(2.0f) == glm::vec3(3.0f, 3.0f, 5.0f));
        REQUIRE(ray.point_at(-1.0f) == glm::vec3(3.0f, 3.0f, 2.0f));
    }

}

TEST_CASE("Ray::intersect(const Plane&)") {

    SECTION("returns the intersection point t") {
        const Plane plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const Ray ray(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        const auto maybe_intersection = ray.intersect(plane);
        REQUIRE(maybe_intersection.has_value());
        REQUIRE(maybe_intersection.value() == 1.0f);
    }
    
}
