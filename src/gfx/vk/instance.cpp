#include "gfx/vk/instance.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace inf::gfx::vk {

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
        std::uint32_t extension_count = 0;
        const char** extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
        instance_create_info.enabledExtensionCount = extension_count;
        instance_create_info.ppEnabledExtensionNames = extension_names;
        instance_create_info.enabledLayerCount = 0;

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