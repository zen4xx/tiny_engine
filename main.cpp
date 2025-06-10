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
    engine.addObject("triangle1", vertices, indices, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    engine.addObject("triangle2", vertices, indices, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    engine.addObject("triangle3", vertices, indices, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));

    glm::vec3 pos(0.0f);
    float angle = 0.0f;
    float speed = 3.0f;

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
    
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        
        engine.moveObject("triangle", model);
        engine.moveObject("triangle1", glm::translate(glm::rotate(glm::mat4(1), (float)sin(glfwGetTime()), glm::vec3(0, 0, 1)), glm::vec3(0, 0, sin(glfwGetTime()))));
        engine.moveObject("triangle2", glm::translate(glm::rotate(glm::mat4(1), (float)cos(glfwGetTime()), glm::vec3(0, 0, 1)), glm::vec3(0, sin(glfwGetTime()), 0)));
        engine.moveObject("triangle3", glm::translate(glm::rotate(glm::mat4(1), (float)tan(glfwGetTime()), glm::vec3(0, 0 ,1)), glm::vec3(sin(glfwGetTime()), 0, 0)));
        std::cout << engine.getFPSCount() << std::endl;
        engine.update();   
    }
    return 0;
}
