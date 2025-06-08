#include "core/core.h"
#include <chrono>

int main()
{
    auto engine = Tiny_engine("app", 800, 600, "test", true);

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    
    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    glm::mat4 view = glm::lookAt(glm::vec3(-2.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    engine.setView(view);

    glm::mat4 matrix(0.f);
    engine.addObject("test", vertices, indices, matrix);

    while (engine.isWindowOpen())
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        glm::mat4 pos = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        engine.moveObject("test", pos);
        engine.update();
    }

    return 0;
}
