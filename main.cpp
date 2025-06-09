#include "core/core.h"

int main()
{
    auto engine = Tiny_engine("app", 800, 600, "test", true);

    std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    std::vector<uint16_t> indices = {
        0, 1, 2};

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    engine.setView(view);

    engine.addObject("triangle", vertices, indices, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));

    glm::vec3 pos(0.0f);
    float angle = 0.0f;

    while (engine.isWindowOpen())
    {
        if (engine.isKeyPressed(GLFW_KEY_W))
            pos.y += 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_S))
            pos.y -= 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_D))
            pos.x += 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_A)) 
            pos.x -= 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_SPACE))
            pos.z += 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
            pos.z -= 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_Q))
            angle += 0.01f;
        if (engine.isKeyPressed(GLFW_KEY_E))
            angle -= 0.01f;
    
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        engine.moveObject("triangle", model);
        engine.update();
    }
    return 0;
}
