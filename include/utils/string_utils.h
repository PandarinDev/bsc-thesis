#pragma once

#include <vector>
#include <string_view>

namespace inf::utils {

    struct StringUtils {

        static std::vector<std::string_view> split(std::string_view input, char delimiter);

        StringUtils() = delete;

    };

}