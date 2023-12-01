#include "window.h"
#include "camera.h"
#include "gfx/renderer.h"

using namespace inf;
using namespace inf::gfx;

int main() {
    Window window("Infinitown", 1600, 900, false);
    Camera camera(glm::vec3(), glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)));
    Renderer renderer(window, camera);

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