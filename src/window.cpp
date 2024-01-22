#include "window.h"

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include <stdexcept>
#include <type_traits>

namespace inf {

    Window::Window(std::string_view title, const WindowSize& window_size, bool full_screen) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("GLFW detected no Vulkan support on host hardware.");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        int width, height;
        std::visit([&width, &height](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, RelativeWindowSize>) {
                const auto monitor = glfwGetPrimaryMonitor();
                const auto video_mode = glfwGetVideoMode(monitor);
                width = static_cast<int>(video_mode->width * value.scale);
                height = static_cast<int>(video_mode->height * value.scale);
            }
            else if constexpr (std::is_same_v<T, FixedWindowSize>) {
                width = value.width;
                height = value.height;
            }
            else {
                throw std::runtime_error("Failed to determine window size, unknown type received.");
            }
        }, window_size);
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