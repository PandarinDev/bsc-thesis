#pragma once

#include "gfx/vk/device.h"

#include <glad/vulkan.h>

#include <vector>

namespace inf::gfx::vk {

    enum class ShaderType {
        VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
        FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    struct Shader {

        static Shader create_from_bytes(const LogicalDevice* device, ShaderType type, const std::vector<char>& bytes);

        Shader(const LogicalDevice* device, ShaderType type, const VkShaderModule& shader);
        ~Shader();
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&);
        Shader& operator=(Shader&&);

        ShaderType get_type() const;
        VkShaderModule get_module() const;

    private:

        const LogicalDevice* device;
        ShaderType type;
        VkShaderModule shader;

    };

}