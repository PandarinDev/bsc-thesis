#pragma once

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string_view>

namespace inf {

    struct Window {

        Window(std::string_view title, int width, int height, bool vsync, bool full_screen);
        ~Window();
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void poll_events() const;
        void swap_buffers() const;
        bool should_close() const;

    private:

        GLFWwindow* handle;

    };

}