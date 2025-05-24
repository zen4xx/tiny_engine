#include "../renderer/renderer.h"
#include "../window/window.h"
#include <memory>

class Tiny_engine{
    public:
        Tiny_engine(const char* application_name, int width, int height, const char* title, GLFWmonitor* monitor=nullptr, GLFWwindow* share=nullptr);
        ~Tiny_engine();

    public:
        inline bool isWindowOpen() { return !glfwWindowShouldClose(m_window.get()->GetWindow()); };
        inline void update() { glfwPollEvents(); };
    private:
        std::unique_ptr<Renderer> m_renderer; 
        std::unique_ptr<Window> m_window;
};
