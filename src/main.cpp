#include "window.h"
#include "gfx/renderer.h"

using namespace inf;
using namespace inf::gfx;

int main() {
    Window window("Infinitown", 1600, 900, true, false);
    Renderer renderer(window);

    while (!window.should_close()) {
        window.poll_events();
        window.swap_buffers();
    }
    glfwTerminate();
    return 0;
}