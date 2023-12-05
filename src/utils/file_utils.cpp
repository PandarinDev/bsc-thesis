#include "utils/file_utils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace inf::utils {

    std::string FileUtils::read_string(const std::filesystem::path& file_path) {
        std::ifstream file_handle(file_path);
        if (!file_handle) {
            throw std::runtime_error("Failed to open file at '" + file_path.string() + "'.");
        }
        std::stringstream str_stream;
        file_handle >> str_stream.rdbuf();
        return str_stream.str();
    }

    std::vector<char> FileUtils::read_bytes(const std::filesystem::path& file_path) {
        std::ifstream file_handle(file_path, std::ios::binary | std::ios::ate);
        if (!file_handle) {
            throw std::runtime_error("Failed to open file at '" + file_path.string() + "'.");
        }
        const auto file_size = file_handle.tellg();
        std::vector<char> bytes(file_size);
        file_handle.seekg(0);
        file_handle.read(bytes.data(), file_size);
        return bytes;
    }

}