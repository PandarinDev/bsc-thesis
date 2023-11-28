#include "gfx/vk/shader.h"

#include <utility>
#include <stdexcept>

namespace inf::gfx::vk {

    Shader Shader::create_from_bytes(const LogicalDevice* device, ShaderType type, const std::vector<char>& bytes) {
        VkShaderModuleCreateInfo shader_create_info{};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_create_info.codeSize = bytes.size();
        shader_create_info.pCode = reinterpret_cast<const std::uint32_t*>(bytes.data());

        VkShaderModule shader;
        if (vkCreateShaderModule(device->get_device(), &shader_create_info, nullptr, &shader) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan shader module.");
        }
        return Shader(device, type, shader);
    }

    Shader::Shader(const LogicalDevice* device, ShaderType type, const VkShaderModule& shader) :
        device(device),
        type(type),
        shader(shader) {}

    Shader::~Shader() {
        if (device) {
            vkDestroyShaderModule(device->get_device(), shader, nullptr);
        }
    }

    Shader::Shader(Shader&& other) :
        device(std::exchange(other.device, nullptr)),
        type(other.type),
        shader(std::exchange(other.shader, VK_NULL_HANDLE)) {}

    Shader& Shader::operator=(Shader&& other) {
        device = std::exchange(other.device, nullptr);
        type = other.type;
        shader = std::exchange(other.shader, VK_NULL_HANDLE);

        return *this;
    }

    ShaderType Shader::get_type() const {
        return type;
    }

    VkShaderModule Shader::get_module() const {
        return shader;
    }

}