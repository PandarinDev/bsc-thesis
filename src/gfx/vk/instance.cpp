#include "gfx/vk/instance.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace inf::gfx::vk {

    static constexpr bool ENABLE_VALIDATION = false;
    static const std::vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    Instance Instance::create_instance(std::string_view application_name) {
        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = application_name.data();
        application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        application_info.pEngineName = "infinitown-engine";
        application_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        application_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;

        // Fetch the required instance extensions from GLFW
        std::uint32_t glfw_extension_count = 0;
        const char** glfw_extension_names = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extension_names;
        for (std::size_t i = 0; i < glfw_extension_count; ++i) {
            extension_names.emplace_back(glfw_extension_names[i]);
        }
        // MoltenVK requires the portability enumeration extension and the portability bit being set as it is not a fully compliant implementation
        #ifdef __APPLE__
        extension_names.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        instance_create_info.enabledExtensionCount = static_cast<std::uint32_t>(extension_names.size());
        instance_create_info.ppEnabledExtensionNames = extension_names.data();
        instance_create_info.enabledLayerCount = 0;

        // Check if validation layers are supported
        if (ENABLE_VALIDATION) {
            std::uint32_t num_layers = 0;
            vkEnumerateInstanceLayerProperties(&num_layers, nullptr);
            std::vector<VkLayerProperties> available_layers(num_layers);
            vkEnumerateInstanceLayerProperties(&num_layers, available_layers.data());

            for (const auto& required : VALIDATION_LAYERS) {
                bool layer_supported = false;
                for (const auto& available : available_layers) {
                    if (strcmp(required, available.layerName) == 0) {
                        layer_supported = true;
                        break;
                    }
                }
                if (!layer_supported) {
                    throw std::runtime_error(std::string("Validation layer '") + required + "' is not supported by host Vulkan implementation.");
                }
            }
            instance_create_info.enabledLayerCount = static_cast<std::uint32_t>(VALIDATION_LAYERS.size());
            instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        }

        std::unique_ptr<VkInstance> instance = std::make_unique<VkInstance>();
        if (vkCreateInstance(&instance_create_info, nullptr, instance.get()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance.");
        }
        return Instance(std::move(instance));
    }

    Instance::Instance(std::unique_ptr<VkInstance> instance) : instance(std::move(instance)) {}

    Instance::~Instance() {
        if (instance) {
            vkDestroyInstance(get_instance(), nullptr);
        }
    }

    Instance::Instance(Instance&& other) : instance(std::move(other.instance)) {}

    Instance& Instance::operator=(Instance&& other) {
        instance = std::move(other.instance);
        return *this;
    }

    const VkInstance& Instance::get_instance() const {
        return *instance.get();
    }

}