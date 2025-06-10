#include "../renderer/renderer.h"
#include "../window/window.h"
#include <memory>

class Tiny_engine
{
public:
    Tiny_engine(const char *application_name, int width, int height, const char *title, bool is_debug = false, GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);
    ~Tiny_engine();

public:
    inline bool isWindowOpen() { return !glfwWindowShouldClose(m_window->GetWindow()); };
    inline void update()
    {
        glfwPollEvents();
        m_renderer->drawScene();
    };
    inline void addObject(std::string name, std::vector<Vertex> vertices, std::vector<uint16_t> indices, glm::mat4 pos) { m_renderer->addObject(name, vertices, indices, pos); };
    inline void moveObject(std::string name, glm::mat4 pos) { m_renderer->moveObject(name, pos); };
    inline void setView(glm::mat4 view) { m_renderer->setView(view); };
    inline bool isKeyPressed(int key) { return glfwGetKey(m_window->GetWindow(), key); };
    inline float getDeltaTime() { return m_renderer->getDeltaTime(); };
    inline float getFPSCount() { return m_renderer->getFPSCount(); };

private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Window> m_window;
};
