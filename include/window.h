#pragma once

#include "gfx/vk/instance.h"
#include "gfx/vk/surface.h"

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <string_view>

namespace inf {

    struct Window {

        Window(std::string_view title, int width, int height, bool full_screen);
        ~Window();
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        glm::ivec2 get_size() const;
        gfx::vk::Surface create_surface(const gfx::vk::Instance& instance) const;

        void poll_events() const;
        bool should_close() const;

    private:

        GLFWwindow* handle;

    };

}