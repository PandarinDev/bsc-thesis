#include "window.h"

#include <stdexcept>

namespace inf {

    Window::Window(std::string_view title, int width, int height, bool vsync, bool full_screen) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("GLFW detected no Vulkan support on host hardware.");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        handle = glfwCreateWindow(width, height, title.data(), full_screen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if (!handle) {
            throw std::runtime_error("Failed to create GLFW window.");
        }
        glfwMakeContextCurrent(handle);
        glfwSwapInterval(vsync ? 1 : 0);
    }

    Window::~Window() {
        glfwDestroyWindow(handle);
    }

    gfx::vk::Surface Window::create_surface(const gfx::vk::Instance& instance) const {
        VkSurfaceKHR surface;

        // TODO: Add Windows and Linux support here
        VkMetalSurfaceCreateInfoEXT surface_create_info{};
        surface_create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        if (vkCreateMetalSurfaceEXT(instance.get_instance(), &surface_create_info, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Metal surface.");
        }

        return gfx::vk::Surface(&instance, surface);
    }

    void Window::poll_events() const {
        glfwPollEvents();
    }

    void Window::swap_buffers() const {
        glfwSwapBuffers(handle);
    }

    bool Window::should_close() const {
        return glfwWindowShouldClose(handle);
    }

}