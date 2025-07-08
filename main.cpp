#include "core/core.h"

int main()
{
    auto engine = Tiny_engine("app", 800, 600, "test", 6, true);

    std::vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0}, {0.0f, 0.0f, 1.0f}}};

    std::vector<uint16_t> indices = {
        0, 1, 2};

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    engine.createScene("main");
    engine.createScene("secondary");

    engine.setView("main", view);
    engine.setView("secondary", view);

    engine.addObject("main", "triangle", vertices, indices, glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));
    engine.addObject("secondary", "triangle", vertices, indices, glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));

    glm::vec3 pos(0.0f);
    float angle = 0.0f;
    float speed = 3.0f;

    bool isMainScene = 1;

    while (engine.isWindowOpen())
    {
        if (engine.isKeyPressed(GLFW_KEY_W))
            pos.y += speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_S))
            pos.y -= speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_D))
            pos.x += speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_A))
            pos.x -= speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_SPACE))
            pos.z += speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
            pos.z -= speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_Q))
            angle += speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_E))
            angle -= speed * engine.getDeltaTime();
        if (engine.isKeyPressed(GLFW_KEY_F))
            std::cout << engine.getFPSCount() << std::endl;
        if (engine.isKeyPressed(GLFW_KEY_X))
            isMainScene ? isMainScene = 0 : isMainScene = 1;

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        engine.update();

        if (isMainScene)
        {
            engine.moveObject("main", "triangle", model);
            engine.drawScene("main");
        }
        else
        {
            engine.moveObject("secondary", "triangle", glm::mat4(1));
            engine.drawScene("secondary");
        }
    }
    return 0;
}
