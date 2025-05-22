#ifndef RENDERER_H
#define RENDERER_H

#include "../scene/scene.h"
#include "../error_handler/error_handler.h"
#include <vector>

//including vulkan
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Renderer{
    public:
        Renderer(const char* app_name);
        ~Renderer();
    public:
        bool RenderScene();
    private:
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
    private:
        VkApplicationInfo m_app_info{};
        VkInstanceCreateInfo m_create_info{};

        VkInstance m_instance;
        VkResult m_result;
};

#endif
