#pragma once

#include <vector>
#include <string_view>

namespace inf::utils {

    struct StringUtils {

        static std::vector<std::string_view> split(std::string_view input, char delimiter);
        static std::string to_uppercase(std::string_view input);

        StringUtils() = delete;

    };

}