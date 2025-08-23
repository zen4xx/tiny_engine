#include "tiny_engine.h"
#include "camera.h"
#include <thread>
#include <iostream>

Camera camera;

double lastX = 400, lastY = 300;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

int main()
{
    uint8_t thread_count = std::thread::hardware_concurrency(); 
    
    auto engine = Tiny_engine("myapp", 1920, 1080, "test", TINY_ENGINE_MAX_MSAA_QUALITY, thread_count, true);
    
    glfwSetCursorPosCallback(engine.getWindow(), mouse_callback);
    glfwSetInputMode(engine.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(engine.getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -4.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view2 = glm::lookAt(glm::vec3(0.0f, -5.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    engine.createScene("main");
    engine.createScene("secondary");

    engine.setDirLight("main", glm::vec3(2.0f, 0.5f, 2.0f));
    engine.setDirLight("secondary", glm::vec3(2.0f, 0.5f, 2.0f));
    
    engine.setDirLightColor("secondary", glm::vec3(5.0f, 10.0f, 10.0f));
    engine.setAmbient("secondary", glm::vec3(5.0f, 10.0f, 10.0f));
    
    engine.setDirLightColor("main", glm::vec3(2.0f, 1.0f, 0.5f));
    engine.setAmbient("main", glm::vec3(0.1f, 0.1f, 0.2f));
    
    engine.setDrawDistance("main", 50.f);
    
    engine.setView("main", view);
    engine.setView("secondary", view2);
    
    tiny_engine::Object suzanne;
    suzanne.scene_name = "secondary";
    suzanne.obj_name = "monkey";
    suzanne.gltf_model_path = "suzanne/Suzanne.gltf";
    suzanne.pos = glm::translate(glm::mat4(1), glm::vec3(0,0,0));
    suzanne.albedo_path = "suzanne/Suzanne_BaseColor.png";
    suzanne.mr_path = "suzanne/Suzanne_MetallicRoughness.png";
    
    engine.addObject(suzanne);
    engine.addObject("main", "helmet", "damaged_helmet/DamagedHelmet.gltf", glm::rotate(glm::mat4(1), glm::radians(-90.f), glm::vec3(1, 0, 0)), "damaged_helmet/Default_albedo.jpg", "damaged_helmet/Default_metalRoughness.jpg", "damaged_helmet/Default_normal.jpg");
    engine.addObject("main", "helmet2", "damaged_helmet/DamagedHelmet.gltf", glm::rotate(glm::mat4(1), glm::radians(-90.f), glm::vec3(1, 0, 0)), "damaged_helmet/Default_albedo.jpg", "damaged_helmet/Default_metalRoughness.jpg", "damaged_helmet/Default_normal.jpg");
    engine.setPointLightsCount("main", 2);
    engine.setPointLight("main", glm::vec3(2.0f, 0.0f, 0.0f), 0);
    engine.setPointLightColor("main", glm::vec3(2.0f, 0.0f, 0.0f), 0);
    engine.setPointLightColor("main", glm::vec3(0.0f, 2.0f, 0.0f), 1);
    
    bool isMainScene = 1;

    glm::vec3 hpos(3.0f, 0.0f, 0.0f);
    float speed = 3.0f;
    
    while (engine.isWindowOpen())
    {
        if (engine.isKeyPressed(GLFW_KEY_W))
            camera.ProcessKeyboard(GLFW_KEY_W, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_S))
            camera.ProcessKeyboard(GLFW_KEY_S, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_D))
            camera.ProcessKeyboard(GLFW_KEY_D, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_A))
            camera.ProcessKeyboard(GLFW_KEY_A, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_F))
            std::cout << engine.getFPSCount() << std::endl;
        if (engine.isKeyPressed(GLFW_KEY_X))
            isMainScene ? isMainScene = 0 : isMainScene = 1;
        if(engine.isKeyPressed(GLFW_KEY_LEFT))
            hpos.x -= speed * engine.getDeltaTime();
        if(engine.isKeyPressed(GLFW_KEY_RIGHT))
            hpos.x += speed * engine.getDeltaTime();
        if(engine.isKeyPressed(GLFW_KEY_DOWN))
            hpos.z += speed * engine.getDeltaTime();
        if(engine.isKeyPressed(GLFW_KEY_UP))
            hpos.z -= speed * engine.getDeltaTime();

        engine.setView("main", camera.GetViewMatrix());

        double t = glfwGetTime();
        glm::vec3 pos(sin(t) * 2 , -1, cos(t) * 2);
        glm::vec3 pos2(cos(t) * 2 , -1, sin(t) * 2);
        engine.setPointLight("main", pos, 0);
        engine.setPointLight("main", pos2, 1);

        
        glm::mat4 model(1.0f);
        model = glm::translate(model, hpos);
        model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
        engine.moveObject("main", "helmet2", model);

        engine.update();

        if (isMainScene)
        {
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
