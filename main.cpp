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
    engine.setView(view);

    engine.addObject("triangle", vertices, indices, glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));    

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
        if (engine.isKeyPressed(GLFW_KEY_F))
            std::cout << engine.getFPSCount() << std::endl;
    
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        engine.moveObject("triangle", model);
        engine.update();   
    }
    return 0;
}
