#include "timer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace inf {

    Timer::Timer() :
        frames(0),
        fps(0),
        last_frame(0.0),
        last_fps(0.0),
        delta(0.0) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        const auto time = glfwGetTime();
        last_frame = time;
        last_fps = time;
    }

    void Timer::tick() {
        ++frames;
        const auto time = glfwGetTime();
        if (time - last_fps >= 1.0) {
            fps = frames;
            frames = 0;
        }
        delta = time - last_frame;
        last_frame = time;
    }

    double Timer::get_time() const {
        return glfwGetTime();
    }

    double Timer::get_delta() const {
        return delta;
    }

}