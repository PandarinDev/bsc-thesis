#include "gfx/vk/device.h"

#include <stdexcept>
#include <utility>
#include <limits>
#include <unordered_set>

namespace inf::gfx::vk {

    static const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

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

    LogicalDevice::LogicalDevice(
        const VkDevice& device,
        const QueueFamilyIndices& queue_family_indices,
        const SwapChainSupport& swap_chain_support) :
        device(device),
        queue_family_indices(queue_family_indices),
        swap_chain_support(swap_chain_support) {}

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(device, nullptr);
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& other) :
        device(std::exchange(other.device, VK_NULL_HANDLE)),
        queue_family_indices(std::move(other.queue_family_indices)),
        swap_chain_support(std::move(other.swap_chain_support)) {}

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& other) {
        device = std::exchange(other.device, VK_NULL_HANDLE);
        queue_family_indices = std::move(other.queue_family_indices);
        swap_chain_support = std::move(other.swap_chain_support);
        return *this;
    }

    const VkDevice& LogicalDevice::get_device() const {
        return device;
    }

    SwapChain LogicalDevice::create_swap_chain([[maybe_unused]] const Surface& surface) const {
        [[maybe_unused]] const auto surface_format = choose_surface_format();
        [[maybe_unused]] const auto present_mode = choose_present_mode();
        [[maybe_unused]] const auto extent = choose_extent();
        
        std::uint32_t image_count = swap_chain_support.surface_capabilities.minImageCount + 1;
        // Max image count = 0 in the surface capability means that there is no maximum
        if (swap_chain_support.surface_capabilities.maxImageCount != 0 &&
            image_count > swap_chain_support.surface_capabilities.maxImageCount) {
            image_count = swap_chain_support.surface_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = surface.get_surface();
        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = surface_format.format;
        swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
        swap_chain_create_info.imageExtent = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_chain_create_info.preTransform = swap_chain_support.surface_capabilities.currentTransform;
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = present_mode;
        swap_chain_create_info.clipped = VK_TRUE;

        // If the graphics and present queues are different we set the sharing mode to concurrent
        if (queue_family_indices.graphics_family.value() != queue_family_indices.presentation_family.value()) {
            std::vector<std::uint32_t> queue_indices = {
                queue_family_indices.graphics_family.value(),
                queue_family_indices.presentation_family.value()
            };
            swap_chain_create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_indices.size());
            swap_chain_create_info.pQueueFamilyIndices = queue_indices.data();
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        }
        else {
            std::vector<std::uint32_t> queue_indices = { queue_family_indices.graphics_family.value() };
            swap_chain_create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_indices.size());
            swap_chain_create_info.pQueueFamilyIndices = queue_indices.data();
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkSwapchainKHR swap_chain;
        if (vkCreateSwapchainKHR(device, &swap_chain_create_info, nullptr, &swap_chain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan swap chain.");
        }
        return SwapChain(swap_chain, this, surface_format.format, extent);
    }

    VkSurfaceFormatKHR LogicalDevice::choose_surface_format() const {
        if (swap_chain_support.surface_formats.empty()) {
            throw std::runtime_error("Cannot choose surface format for a physical device that doesn't support any.");
        }
        for (const auto& format : swap_chain_support.surface_formats) {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB) {
                return format;
            }
        }
        return swap_chain_support.surface_formats[0];
    }

    VkPresentModeKHR LogicalDevice::choose_present_mode() const {
        if (swap_chain_support.present_modes.empty()) {
            throw std::runtime_error("Cannot choose present mode for a physical device that doesn't support any.");
        }
        for (const auto& mode : swap_chain_support.present_modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D LogicalDevice::choose_extent() const {
        const auto& capabilities = swap_chain_support.surface_capabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        // TODO
        throw std::runtime_error("Unsupported window manager.");
    }

    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& device, const Surface& surface) :
        device(device), supports_required_extensions(false) {
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

        // Check if the device supports all the required extensions
        bool supports_required_extensions = true;
        std::uint32_t num_supported_extensions = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &num_supported_extensions, nullptr);
        std::vector<VkExtensionProperties> supported_extensions(num_supported_extensions);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &num_supported_extensions, supported_extensions.data());
        for (const auto& required : REQUIRED_DEVICE_EXTENSIONS) {
            bool extension_supported = false;
            for (const auto& supported : supported_extensions) {
                if (strcmp(required, supported.extensionName) == 0) {
                    extension_supported = true;
                    break;
                }
            }
            if (!extension_supported) {
                supports_required_extensions = false;
                break;
            }
        }
        this->supports_required_extensions = supports_required_extensions;
    }

    const VkPhysicalDevice& PhysicalDevice::get_physical_device() const {
        return device;
    }

    bool PhysicalDevice::is_dedicated_gpu() const {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool PhysicalDevice::is_suitable(const Surface& surface) const {
        bool has_queue_families_and_extensions = queue_family_indices.is_complete() && supports_required_extensions;
        if (!has_queue_families_and_extensions) {
            return false;
        }
        const auto swap_chain_support = query_swap_chain_support(surface);
        if (swap_chain_support.surface_formats.empty() || swap_chain_support.present_modes.empty()) {
            return false;
        }

        return true;
    }

    const QueueFamilyIndices& PhysicalDevice::get_queue_family_indices() const {
        return queue_family_indices;
    }

    LogicalDevice PhysicalDevice::create_logical_device(const Surface& surface) const {
        const auto queue_create_info = queue_family_indices.to_queue_create_info();
        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_info.data();
        device_create_info.queueCreateInfoCount = queue_create_info.size();
        device_create_info.pEnabledFeatures = &device_features;

        #ifdef __APPLE__
        std::vector<const char*> enabled_extensions;
        enabled_extensions.insert(enabled_extensions.cend(), REQUIRED_DEVICE_EXTENSIONS.cbegin(), REQUIRED_DEVICE_EXTENSIONS.cend());
        enabled_extensions.emplace_back("VK_KHR_portability_subset");
        device_create_info.enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size());
        device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
        #else
        device_create_info.enabledExtensionCount = static_cast<std::uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
        device_create_info.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();
        #endif
        // TODO: The device layer count is likely ignored by modern implementation, but should still be set for compatibility reasons
        device_create_info.enabledLayerCount = 0;

        VkDevice logical_device;
        if (vkCreateDevice(device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical Vulkan device.");
        }

        const auto swap_chain_support = query_swap_chain_support(surface);
        return LogicalDevice(logical_device, queue_family_indices, swap_chain_support);
    }

    SwapChainSupport PhysicalDevice::query_swap_chain_support(const Surface& surface) const {
        SwapChainSupport swap_chain_support;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.get_surface(), &swap_chain_support.surface_capabilities);

        // Initialize surface formats
        std::uint32_t num_surface_format = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get_surface(), &num_surface_format, nullptr);
        swap_chain_support.surface_formats.resize(num_surface_format);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.get_surface(), &num_surface_format, swap_chain_support.surface_formats.data());

        // Initialize present modes
        std::uint32_t num_present_modes = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get_surface(), &num_present_modes, nullptr);
        swap_chain_support.present_modes.resize(num_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.get_surface(), &num_present_modes, swap_chain_support.present_modes.data());

        return swap_chain_support;
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
            if (device.is_dedicated_gpu() && device.is_suitable(surface)) {
                return device;
            }
        }

        // Failing to find any dedicated GPUs, we fail back to the first integrated one that is suitable.
        // TODO: Instead of iterating again here we should cache the first suitable device and use that
        for (const auto& raw_device : devices) {
            PhysicalDevice device(raw_device, surface);
            if (device.is_suitable(surface)) {
                return device;
            }
        }
        throw std::runtime_error("No suitable GPUs found on the host hardware.");
    }

}