#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <stdexcept>
#include <limits>

namespace inf::utils {

    struct MathUtils {

        MathUtils() = delete;

        // Generic lerp: Works for any number of values, not just two.
        template<typename T>
        static T lerp(const std::vector<T>& values, float t) {
            if (values.size() < 2) {
                throw std::runtime_error("Number of values passed to lerp() must be at least 2.");
            }
            t = glm::clamp(t, 0.0f, 1.0f);
            const float t_step = (1.0f / static_cast<float>(values.size() - 1));
            const std::size_t first_idx = static_cast<std::size_t>(t / (t_step + std::numeric_limits<float>::epsilon()));
            const std::size_t second_idx = first_idx + 1;
            const float normalized_t = (t - first_idx * t_step) / t_step;
            return glm::mix(values.at(first_idx), values.at(second_idx), normalized_t);
        }

    };

}