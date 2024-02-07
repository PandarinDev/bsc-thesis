#include "utils/string_utils.h"

#include <catch2/catch_test_macros.hpp>

using namespace inf::utils;

TEST_CASE("StringUtils::split()") {

    SECTION("Returns empty vector for empty input") {
        REQUIRE(StringUtils::split("", ' ').empty());
    }

    SECTION("Returns vector with entire substring if input has no delimiters") {
        const auto result = StringUtils::split("foobar", ' ');
        REQUIRE(result.size() == 1);
        REQUIRE(result.at(0) == "foobar");
    }

    SECTION("Skips over empty sections between delimiters") {
        const auto result = StringUtils::split("foo    ba  r", ' ');
        REQUIRE(result.size() == 3);
        REQUIRE(result.at(0) == "foo");
        REQUIRE(result.at(1) == "ba");
        REQUIRE(result.at(2) == "r");
    }

    SECTION("Handles (multiple) delimiters at the beginning of the input") {
        const auto result = StringUtils::split("--foobar", '-');
        REQUIRE(result.size() == 1);
        REQUIRE(result.at(0) == "foobar");
    }

    SECTION("Handles (multiple) delimiters at the end of the input") {
        const auto result = StringUtils::split("foobar--", '-');
        REQUIRE(result.size() == 1);
        REQUIRE(result.at(0) == "foobar");
    }

}

TEST_CASE("StringUtils::to_uppercase()") {

    SECTION("Uppercases the input string") {
        REQUIRE(StringUtils::to_uppercase("foobar") == "FOOBAR");
    }

}
