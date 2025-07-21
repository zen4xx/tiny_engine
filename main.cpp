#include "core/core.h"
#include <bits/stdc++.h>

int main()
{
    auto engine = Tiny_engine("app", 800, 600, "test", 6, true);

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0};

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    engine.createScene("main");
    engine.createScene("secondary");

    engine.setView("main", view);
    engine.setView("secondary", view);

    engine.addObject("main", "grid", vertices, indices, glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)), "texture.jpg");
    engine.addObject("secondary", "grid", vertices, indices, glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)), "texture.jpg");

    glm::vec3 pos(0.0f);
    float angle = 0.0f;
    float speed = 3.0f;

    bool isMainScene = 1;
    int cnt = 1;

    // For random generation triangles
    std::default_random_engine gen;
    std::uniform_real_distribution<double> distribution(-5.0, 5.0);

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
        if (engine.isKeyPressed(GLFW_KEY_Z))
        {
            engine.addObject("main", "grid" + std::to_string(++cnt), vertices, indices, glm::translate(glm::mat4(1), glm::vec3(distribution(gen), distribution(gen), distribution(gen))));
            engine.updateScene("main");
        }
        if (engine.isKeyPressed(GLFW_KEY_C))
            std::cout << cnt << std::endl;

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        engine.update();

        if (isMainScene)
        {
            engine.moveObject("main", "grid", model);
            engine.drawScene("main");
        }
        else
        {
            glm::mat4 secondary_model = glm::translate(glm::mat4(1), glm::vec3(sin(glfwGetTime()), cos(glfwGetTime()), 0));
            secondary_model = glm::rotate(secondary_model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            engine.moveObject("secondary", "grid", secondary_model); 
            engine.drawScene("secondary");
        }
    }
    return 0;
}
