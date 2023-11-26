#include "gfx/vk/device.h"

#include <stdexcept>
#include <utility>
#include <unordered_set>

namespace inf::gfx::vk {

    bool QueueFamilyIndices::is_complete() const {
        return graphics_family.has_value() && presentation_family.has_value();
    }

    std::vector<VkDeviceQueueCreateInfo> QueueFamilyIndices::to_queue_create_info() const {
        static const float GENERAL_QUEUE_PRIORITY = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::unordered_set<std::uint32_t> unique_queue_families = { graphics_family.value(), presentation_family.value() };
        for (const auto queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo graphics_queue_create_info{};
            graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphics_queue_create_info.queueFamilyIndex = queue_family;
            graphics_queue_create_info.queueCount = 1;
            graphics_queue_create_info.pQueuePriorities = &GENERAL_QUEUE_PRIORITY;
            queue_create_infos.emplace_back(std::move(graphics_queue_create_info));
        }

        return queue_create_infos;
    }

    LogicalDevice::LogicalDevice(const VkDevice& device, const QueueFamilyIndices& queue_family_indices) :
        device(device), queue_family_indices(queue_family_indices) {}

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(device, nullptr);
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& other) :
        device(std::exchange(other.device, nullptr)),
        queue_family_indices(std::move(other.queue_family_indices)) {}

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& other) {
        device = std::exchange(other.device, nullptr);
        queue_family_indices = std::move(other.queue_family_indices);
        return *this;
    }

    VkQueue LogicalDevice::get_graphics_queue() const {
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, queue_family_indices.graphics_family.value(), 0, &graphics_queue);
        return graphics_queue;
    }

    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& device, const Surface& surface) : device(device) {
        vkGetPhysicalDeviceProperties(device, &properties);

        // Build queue family indices
        std::uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties.data());
        for (std::size_t i = 0; i < queue_family_properties.size(); ++i) {
            const auto& queue_family = queue_family_properties[i];
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queue_family_indices.graphics_family = i;
            }

            VkBool32 presentation_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface.get_surface(), &presentation_support);
            if (presentation_support) {
                queue_family_indices.presentation_family = i;
            }
        }
    }

    bool PhysicalDevice::is_dedicated_gpu() const {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool PhysicalDevice::is_suitable() const {
        return queue_family_indices.is_complete();
    }

    const QueueFamilyIndices& PhysicalDevice::get_queue_family_indices() const {
        return queue_family_indices;
    }

    LogicalDevice PhysicalDevice::create_logical_device() const {
        const auto queue_create_info = queue_family_indices.to_queue_create_info();
        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_info.data();
        device_create_info.queueCreateInfoCount = queue_create_info.size();
        device_create_info.pEnabledFeatures = &device_features;
        device_create_info.enabledExtensionCount = 0;
        // NOTE: The device layer count is likely ignored by modern implementation, but should still be set for compatibility reasons
        device_create_info.enabledLayerCount = 0;

        VkDevice logical_device;
        if (vkCreateDevice(device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical Vulkan device.");
        }
        return LogicalDevice(logical_device, queue_family_indices);
    }

    PhysicalDevice Device::choose_optimal_device(const Instance& instance, const Surface& surface) {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, nullptr);
        if (device_count == 0) {
            throw std::runtime_error("No physical Vulkan devices found.");
        }
        
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, devices.data());
        // Currently we are simply choosing the first dedicated GPU that is suitable.
        for (const auto& raw_device : devices) {
            PhysicalDevice device(raw_device, surface);
            if (device.is_suitable() && device.is_dedicated_gpu()) {
                return device;
            }
        }

        // Failing to find any dedicated GPUs, we fail back to the first integrated one that is suitable.
        for (const auto& raw_device : devices) {
            PhysicalDevice device(raw_device, surface);
            if (device.is_suitable()) {
                return device;
            }
        }
        throw std::runtime_error("No suitable GPUs found on the host hardware.");
    }

}