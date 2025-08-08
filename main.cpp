#include "tiny_engine.h"

int main()
{
    uint8_t thread_count = std::thread::hardware_concurrency(); 

    auto engine = Tiny_engine("app", 800, 600, "test", TINY_ENGINE_MAX_MSAA_QUALITY, thread_count, true);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -4.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view2 = glm::lookAt(glm::vec3(0.0f, -5.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    engine.createScene("main");
    engine.createScene("secondary");

    engine.setDirLight("main", glm::vec3(2.0f, 0.0f, -2.0f));
    engine.setDirLight("secondary", glm::vec3(2.0f, 0.0f, -2.0f));

    engine.setDirLightColor("secondary", glm::vec3(1.0f, 0.0f, 0.0f));
    engine.setAmbient("secondary", glm::vec3(0.0f, 0.0f, 0.3f));

    engine.setDrawDistance("main", 50.f);

    engine.setView("main", view);
    engine.setView("secondary", view2);

    tiny_engine::Object suzanne;
    suzanne.scene_name = "secondary";
    suzanne.obj_name = "monkey";
    suzanne.gltf_model_path = "suzanne/Suzanne.gltf";
    suzanne.pos = glm::translate(glm::mat4(1), glm::vec3(0,0,0));
    suzanne.texture_path = "suzanne/Suzanne_BaseColor.png";
    
    engine.addObject("main", "helmet", "damaged_helmet/DamagedHelmet.gltf", glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)), "damaged_helmet/Default_albedo.jpg");
    engine.addObject(suzanne);
   
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

        engine.setDirLight("main", glm::vec3(sin(glfwGetTime()) * 3.0f, 0.0f, -2.0f));

        engine.update();
        

        if (isMainScene)
        {
            engine.moveObject("main", "helmet", model);
            engine.drawScene("main");
        }
        else
        {
            glm::mat4 secondary_model = glm::translate(glm::mat4(1), glm::vec3(sin(glfwGetTime()), cos(glfwGetTime()), 0));
            secondary_model = glm::rotate(secondary_model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            secondary_model = glm::rotate(secondary_model, -1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
            engine.moveObject(suzanne, secondary_model);
            engine.drawScene("secondary");
        }
    }
    return 0;
}
