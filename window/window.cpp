#include "window.h"

Window::Window(int width, int height, const char* name, GLFWmonitor* monitor, GLFWwindow* share){
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, name, monitor, share);
}

Window::~Window(){
    glfwDestroyWindow(m_window);
}

GLFWwindow* Window::GetWindow(){
    return m_window;
}
