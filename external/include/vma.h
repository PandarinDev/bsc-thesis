// Disable both static and dynamic function lookup as we'll use GLAD instead
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
// TODO: This should be raised to Vulkan 1.3 probably, but needs some extensions to be enabled in GLAD
#define VMA_VULKAN_VERSION 1000000
#include "vk_mem_alloc.h"