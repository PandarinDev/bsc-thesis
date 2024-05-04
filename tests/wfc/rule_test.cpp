#include "wfc/rule.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>
#include <optional>
#include <functional>

using namespace inf;
using namespace inf::wfc;

// Utility classes used for WFC testing

enum class CellColor {
    RED,
    BLUE
};

struct TestCell {

    bool even;
    std::optional<CellColor> color;

    TestCell(bool even) : even(even) {}

};

struct TestContext {

    using InstanceType = TestCell;

    std::vector<TestCell> cells;

};

struct ColorRule {

    using Predicate = std::function<bool(const TestCell&)>;

    Predicate predicate;
    CellColor color;

    ColorRule(const Predicate& predicate, CellColor color) : predicate(predicate), color(color) {}

    bool matches(const TestContext&, const TestCell& cell) const {
        return predicate(cell);
    }

    void apply(TestContext&, TestCell& cell) const {
        cell.color = color;
    }

};

TEST_CASE("wfc_collapse()") {

    SECTION("Runs WFC algorithm") {
        TestContext context;
        // Place 50 cells into the context
        for (std::size_t i = 0; i < 50; ++i) {
            // Set even = true if i is divisible by 2
            context.cells.emplace_back(i % 2 == 0);
        }
        // Color the cells of the context to red if the cell is even or blue otherwise
        // Random generator is not used for anything here as the rules are mutually exclusive
        RandomGenerator random_generator(42);
        const auto is_even = [](const TestCell& cell) { return cell.even; };
        const auto is_odd = [](const TestCell& cell) { return !cell.even; };
        wfc_collapse(random_generator, context, std::vector<ColorRule>{
            ColorRule(is_even, CellColor::RED),
            ColorRule(is_odd, CellColor::BLUE)
        });
        for (const auto& cell : context.cells) {
            REQUIRE(cell.color.has_value());
            const auto expected_color = cell.even ? CellColor::RED : CellColor::BLUE;
            REQUIRE(cell.color.value() == expected_color);
        }
    }

}