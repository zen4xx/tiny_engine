#include "window.h"

Window::Window(int width, int height, const char* name, GLFWmonitor* monitor, GLFWwindow* share){
    
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, name, monitor, share);
    
    if(!m_window){
        err("cannot create a window", 1);
    }
}

Window::~Window(){
    glfwDestroyWindow(m_window);
}
