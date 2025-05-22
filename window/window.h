#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window{
    public:
        Window(int width, int height, const char* name);
        ~Window();
    public:
        GLFWwindow* GetWindow();
    private:
        GLFWwindow* m_window = nullptr;
};
