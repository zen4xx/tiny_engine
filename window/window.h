#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../error_handler/error_handler.h"

class Window{
    public:
        Window(int width, int height, const char* name, GLFWmonitor* monitor, GLFWwindow* share);
        ~Window();
    public:
        inline GLFWwindow* GetWindow() { return m_window; }
    private:
        GLFWwindow* m_window = nullptr;
};
#endif