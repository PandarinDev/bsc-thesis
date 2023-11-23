#include "window.h"
#include "renderer.h"

using namespace inf;

int main() {
    Window window("Infinitown", 1600, 900, true, false);
    Renderer renderer;
    while (!window.should_close()) {
        window.poll_events();
        window.swap_buffers();
    }
    glfwTerminate();
    return 0;
}