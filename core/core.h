#include "../renderer/renderer.h"
#include "../window/window.h"
#include "GLFW/glfw3.h"
#include <cstddef>
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
    // Returns a object_id
    inline size_t addObject(size_t scene_id, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default") { return m_renderer->addObject(scene_id, vertices, indices, pos, albedo_path, mr_path, normal_path); };
    inline size_t addObject(size_t scene_id, const std::string &gltf_model_path, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default") { return m_renderer->addObject(scene_id, gltf_model_path, pos, albedo_path, mr_path, normal_path); };
    inline size_t addObject(tiny_engine::Object &obj) { return m_renderer->addObject(obj); };

    inline void deleteObject(size_t scene_id, size_t obj_id) { m_renderer->deleteObject(scene_id, obj_id); }; 
    inline void deleteObject(const tiny_engine::Object &obj) { m_renderer->deleteObject(obj); }; 
    
    inline void moveObject(size_t scene_id, size_t obj_id, glm::mat4 pos) { m_renderer->moveObject(scene_id, obj_id, pos); };
    inline void moveObject(const tiny_engine::Object &obj, glm::mat4 pos) { m_renderer->moveObject(obj, pos); };

    inline void drawScene(size_t scene_id) { m_renderer->drawScene(scene_id); };
    // return scene_id
    inline size_t createScene(const std::string &skybox_path="_default") { return m_renderer->createScene(skybox_path); };
    // If objects are added to the scene, the function must be called before drawScene() 
    inline void updateScene(size_t scene_id) { m_renderer->updateScene(scene_id); };

    inline void setView(size_t scene_id, glm::mat4 view) { m_renderer->setView(scene_id, view); };
    inline void setDrawDistance(size_t scene_id, float distance) { m_renderer->setDrawDistance(scene_id, distance); };

    inline void setAmbient(size_t scene_id, glm::vec3 ambient) {m_renderer->setAmbient(scene_id, ambient); };
    inline void setDirLight(size_t scene_id, glm::vec3 dir) { m_renderer->setDirLight(scene_id, dir); };
    inline void setDirLightColor(size_t scene_id, glm::vec3 color) { m_renderer->setDirLightColor(scene_id, color); };
    inline void setPointLight(size_t scene_id, glm::vec3 pos, uint8_t index) { m_renderer->setPointLight(scene_id, pos, index); };
    inline void setPointLightColor(size_t scene_id, glm::vec3 color, uint8_t index) { m_renderer->setPointLightColor(scene_id, color, index); };
    inline void setPointLightsCount(size_t scene_id, uint8_t count) { m_renderer->setPointLightsCount(scene_id, count); }

    inline bool isKeyPressed(int key) { return glfwGetKey(m_window->GetWindow(), key); };

    inline float getDeltaTime() { return m_renderer->getDeltaTime(); };
    inline float getFPSCount() { return m_renderer->getFPSCount(); };
    inline GLFWwindow* getWindow() { return m_window->GetWindow(); };
private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Window> m_window;
};
