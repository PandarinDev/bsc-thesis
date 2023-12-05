#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace inf::utils {

    struct FileUtils {

        FileUtils() = delete;

        static std::string read_string(const std::filesystem::path& file_path);
        static std::vector<char> read_bytes(const std::filesystem::path& file_path);

    };

}