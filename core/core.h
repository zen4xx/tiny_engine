#include "../renderer/renderer.h"
#include "../window/window.h"
#include "GLFW/glfw3.h"
#include <memory>

class Tiny_engine
{
public:
    Tiny_engine(const char *application_name, int width, int height, const char *title, uint8_t msaa_quality = TINY_ENGINE_MAX_MSAA_QUALITY, uint8_t thread_count = 6, bool is_debug = false, GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);
    ~Tiny_engine();

public:
    inline bool isWindowOpen() { return !glfwWindowShouldClose(m_window->GetWindow()); };
    // window, physics, etc..
    inline void update() { glfwPollEvents(); };

    // Must be called before drawScene
    inline void addObject(const std::string &scene_name, const std::string &obj_name, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default") { m_renderer->addObject(scene_name, obj_name, vertices, indices, pos, albedo_path, mr_path, normal_path); };
    inline void addObject(const std::string &scene_name, const std::string &obj_name, const std::string &gltf_model_path, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default") { m_renderer->addObject(scene_name, obj_name, gltf_model_path, pos, albedo_path, mr_path, normal_path); };
    inline void addObject(const tiny_engine::Object &obj) { m_renderer->addObject(obj); };

    inline void deleteObject(const std::string &scene_name, const std::string &obj_name) { m_renderer->deleteObject(scene_name, obj_name); }; 
    inline void deleteObject(const tiny_engine::Object &obj) { m_renderer->deleteObject(obj); }; 
    
    inline void moveObject(const std::string &scene_name, const std::string &obj_name, glm::mat4 pos) { m_renderer->moveObject(scene_name, obj_name, pos); };
    inline void moveObject(const tiny_engine::Object &obj, glm::mat4 pos) { m_renderer->moveObject(obj, pos); };

    inline void drawScene(const std::string &scene_name) { m_renderer->drawScene(scene_name); };
    inline void createScene(const std::string &scene_name) { m_renderer->createScene(scene_name); };
    // If objects are added to the scene, the function must be called before drawScene() 
    inline void updateScene(const std::string& scene_name) { m_renderer->updateScene(scene_name); };

    inline void setView(const std::string &scene_name, glm::mat4 view) { m_renderer->setView(scene_name, view); };
    inline void setDrawDistance(const std::string &scene_name, float distance) { m_renderer->setDrawDistance(scene_name, distance); };

    inline void setAmbient(const std::string &scene_name, glm::vec3 ambient) {m_renderer->setAmbient(scene_name, ambient); };
    inline void setDirLight(const std::string &scene_name, glm::vec3 dir) { m_renderer->setDirLight(scene_name, dir); };
    inline void setDirLightColor(const std::string &scene_name, glm::vec3 color) { m_renderer->setDirLightColor(scene_name, color); };
    inline void setPointLight(const std::string &scene_name, glm::vec3 pos, uint8_t index) { m_renderer->setPointLight(scene_name, pos, index); };
    inline void setPointLightColor(const std::string &scene_name, glm::vec3 color, uint8_t index) { m_renderer->setPointLightColor(scene_name, color, index); };
    inline void setPointLightsCount(const std::string &scene_name, uint8_t count) { m_renderer->setPointLightsCount(scene_name, count); }

    inline bool isKeyPressed(int key) { return glfwGetKey(m_window->GetWindow(), key); };

    inline float getDeltaTime() { return m_renderer->getDeltaTime(); };
    inline float getFPSCount() { return m_renderer->getFPSCount(); };
    inline GLFWwindow* getWindow() { return m_window->GetWindow(); };
private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Window> m_window;
};
