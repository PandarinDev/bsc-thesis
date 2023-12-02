#include "window.h"
#include "camera.h"
#include "timer.h"
#include "gfx/renderer.h"

using namespace inf;
using namespace inf::gfx;

int main() {
    Window window("Infinitown", 1600, 900, false);
    Timer timer;
    Camera camera(glm::vec3(), glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)));
    Renderer renderer(window, camera);

    // Build a test cube that we are going to render
    auto cube = Cube::build(renderer.get_physical_device(), &renderer.get_logical_device(), 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    while (!window.should_close()) {
        timer.tick();
        const auto time = static_cast<float>(timer.get_time());

        // Rotate the cube around
        const auto model_matrix = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)), time, glm::vec3(1.0f, 1.0f, 0.0f));
        cube.set_model_matrix(model_matrix);

        window.poll_events();
        renderer.begin_frame();
        renderer.render(cube);
        renderer.end_frame();
    }
    // Wait until the device becomes idle (flushes queues) to destroy in a well-defined state
    renderer.get_logical_device().wait_until_idle();
    glfwTerminate();
    return 0;
}