#include "window.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <stdexcept>

namespace inf {

    Window::Window(std::string_view title, int width, int height, bool full_screen) {
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
        glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    Window::~Window() {
        glfwDestroyWindow(handle);
    }

    GLFWwindow* Window::get_handle() const {
        return handle;
    }

    glm::ivec2 Window::get_size() const {
        glm::ivec2 size;
        glfwGetWindowSize(handle, &size.x, &size.y);
        return size;
    }

    gfx::vk::Surface Window::create_surface(const gfx::vk::Instance& instance) const {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance.get_instance(), handle, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan surface from GLFW window.");
        }

        return gfx::vk::Surface(&instance, surface);
    }

    void Window::poll_events() const {
        glfwPollEvents();
    }

    bool Window::should_close() const {
        return glfwWindowShouldClose(handle);
    }

}