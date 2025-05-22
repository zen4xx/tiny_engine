#include "window.h"

Window::Window(int width, int height, const char* name){
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

Window::~Window(){
    glfwDestroyWindow(m_window);
}

GLFWwindow* Window::GetWindow(){
    return m_window;
}
