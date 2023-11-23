#include "gfx/vk/device.h"

#include <vector>
#include <stdexcept>

namespace inf::gfx::vk {

    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& device) : device(device) {
        vkGetPhysicalDeviceProperties(device, &properties);
    }

    bool PhysicalDevice::is_dedicated_gpu() const {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    VkPhysicalDevice Device::choose_optimal_device(const Instance& instance) {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, nullptr);
        if (device_count == 0) {
            throw std::runtime_error("No physical Vulkan devices found.");
        }
        
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, devices.data());
        // Currently we are simply choosing the first dedicated GPU that we find.
        // Failing to find any dedicated GPUs, we fail back to the first integrated one.
        for (const auto& raw_device : devices) {
            PhysicalDevice device(raw_device);
            if (device.is_dedicated_gpu()) {
                return raw_device;
            }
        }
        return devices[0];
    }

}