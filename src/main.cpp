#include "window.h"
#include "gfx/renderer.h"

using namespace inf;
using namespace inf::gfx;

int main() {
    Window window("Infinitown", 1600, 900, false);
    Renderer renderer(window);

    while (!window.should_close()) {
        window.poll_events();
        renderer.begin_frame();
        renderer.end_frame();
    }
    // Wait until the device becomes idle (flushes queues) to destroy in a well-defined state
    renderer.get_device().wait_until_idle();
    glfwTerminate();
    return 0;
}