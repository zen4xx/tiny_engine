#include "../renderer/renderer.h"
#include "../window/window.h"
#include <memory>

class Tiny_engine
{
public:
    Tiny_engine(const char *application_name, int width, int height, const char *title, uint8_t thread_count = 6, bool is_debug = false, GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);
    ~Tiny_engine();

public:
    inline bool isWindowOpen() { return !glfwWindowShouldClose(m_window->GetWindow()); };
    // window, physics, etc..
    inline void update() { glfwPollEvents(); };

    // Must be called before drawScene
    inline void addObject(const std::string &scene_name, const std::string &obj_name, std::vector<Vertex> vertices, std::vector<uint16_t> indices, glm::mat4 pos, const std::string &texture_path="_default") { m_renderer->addObject(scene_name, obj_name, vertices, indices, pos, texture_path); };
    inline void addObject(const std::string &scene_name, const std::string &obj_name, const std::string &gltf_model_path, glm::mat4 pos, const std::string &texture_path="_default") { m_renderer->addObject(scene_name, obj_name, gltf_model_path, pos, texture_path); };
    inline void moveObject(const std::string &scene_name, const std::string &obj_name, glm::mat4 pos) { m_renderer->moveObject(scene_name, obj_name, pos); };
    

    inline void drawScene(const std::string &scene_name) { m_renderer->drawScene(scene_name); };
    inline void createScene(const std::string &scene_name) { m_renderer->createScene(scene_name); };
    // If objects are added to the scene, the function must be called before drawScene() 
    inline void updateScene(const std::string& scene_name) { m_renderer->updateScene(scene_name); };

    inline void setView(const std::string &scene_name, glm::mat4 view) { m_renderer->setView(scene_name, view); };
    
    inline bool isKeyPressed(int key) { return glfwGetKey(m_window->GetWindow(), key); };

    inline float getDeltaTime() { return m_renderer->getDeltaTime(); };
    inline float getFPSCount() { return m_renderer->getFPSCount(); };

private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Window> m_window;
};
