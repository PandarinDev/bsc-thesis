#pragma once

#include "XoshiroCpp.h"

#include <random>
#include <type_traits>

namespace inf {

    template<typename T>
    struct Range2D {

        T min;
        T max;

        Range2D(const T& min, const T& max) : min(min), max(max) {}

        std::enable_if_t<std::is_integral_v<T>, std::uniform_int_distribution<T>> to_distribution() const {
            return std::uniform_int_distribution<T>(min, max);
        }

        bool is_inside(const T& value) const {
            return value >= min && value <= max;
        }

    };

    using RandomGenerator = XoshiroCpp::Xoshiro256Plus;

}