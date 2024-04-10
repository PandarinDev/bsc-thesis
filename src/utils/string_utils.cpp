#include "utils/string_utils.h"

#include <cctype>
#include <algorithm>
#include <string>

namespace inf::utils {

    std::vector<std::string_view> StringUtils::split(std::string_view input, char delimiter) {
        std::vector<std::string_view> result;
        std::size_t start_idx = 0;
        for (std::size_t i = 0; i < input.size(); ++i) {
            if (input[i] == delimiter) {
                std::size_t length = i - start_idx;
                if (length > 0) {
                    result.emplace_back(input.substr(start_idx, length));
                }
                start_idx = i + 1;
            }
        }
        if (start_idx < input.length()) {
            result.emplace_back(input.substr(start_idx));
        }

        return result;
    }

    std::string StringUtils::to_uppercase(std::string_view input) {
        std::string result(input);
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

}