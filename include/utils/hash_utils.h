#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <functional>

template<typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<>
struct std::hash<glm::ivec2> {

    std::size_t operator()(const glm::ivec2& vec) const noexcept {
        std::size_t result = std::hash<int>{}(vec.x);
        hash_combine(result, vec.y);
        return result;
    }

};

template<>
struct std::hash<glm::ivec3> {

    std::size_t operator()(const glm::ivec3& vec) const noexcept {
        std::size_t result = std::hash<int>{}(vec.x);
        hash_combine(result, vec.y);
        hash_combine(result, vec.z);
        return result;
    }

};
