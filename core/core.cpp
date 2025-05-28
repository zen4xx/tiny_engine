#include "core.h"

//glfwInit is called in the window constructor (the Window constructor is called before all others) and is terminated in the Tiny_engine destructor
Tiny_engine::Tiny_engine(const char* applictaion_name, int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* window){
    glfwInit();

    m_window = std::make_unique<Window>(width, height, title, monitor, window);
    m_renderer = std::make_unique<Renderer>(applictaion_name);
    m_renderer.get()->setWindow(m_window.get()->GetWindow());
}

Tiny_engine::~Tiny_engine(){
    m_renderer.reset(nullptr);
    m_window.reset(nullptr);

    glfwTerminate();
}
