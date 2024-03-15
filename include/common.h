#pragma once

#include "XoshiroCpp.h"

#include <random>
#include <type_traits>

namespace inf {

    template<typename T>
    struct Range2D {

        T min;
        T max;

        std::enable_if_t<std::is_integral_v<T>, std::uniform_int_distribution<T>> to_distribution() {
            return std::uniform_int_distribution<T>(min, max);
        }

    };

    using RandomGenerator = XoshiroCpp::Xoshiro256Plus;

}