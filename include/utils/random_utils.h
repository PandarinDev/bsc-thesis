#pragma once

#include "common.h"

#include <deque>
#include <cstdint>
#include <numeric>
#include <string>
#include <stdexcept>

namespace inf::utils {

    struct RandomUtils {

        RandomUtils() = delete;

        template<typename T>
        static std::vector<const T*> choose(RandomGenerator& rng, const std::vector<T> from, std::size_t choices) {
            if (choices > from.size()) {
                throw std::runtime_error("Number of choices (" + std::to_string(choices) +
                    ") higher than input collection size (" + std::to_string(from.size()) + ").");
            }
            std::vector<const T*> result;
            std::deque<std::size_t> indices(from.size());
            std::iota(indices.begin(), indices.end(), 0);
            for (std::size_t i = 0; i < choices; ++i) {
                std::uniform_int_distribution<std::size_t> distribution(0, indices.size() - 1);
                const auto index = distribution(rng);
                result.emplace_back(&from[indices[index]]);
                indices.erase(indices.cbegin() + index);
            }
            return result;
        }

    };

}