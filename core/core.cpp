#include "core.h"

Tiny_engine::Tiny_engine(const char *applictaion_name, int width, int height, const char *title, uint8_t thread_count, bool is_debug, GLFWmonitor *monitor, GLFWwindow *window)
{
    glfwInit();
    m_window = std::make_unique<Window>(width, height, title, monitor, window);
    m_renderer = std::make_unique<Renderer>(applictaion_name, is_debug);
    m_renderer->setThreadCount(thread_count);
    m_renderer->setWindow(m_window->GetWindow());
}

Tiny_engine::~Tiny_engine()
{
    m_renderer.reset(nullptr);
    m_window.reset(nullptr);

    glfwTerminate();
}
