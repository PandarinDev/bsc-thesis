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