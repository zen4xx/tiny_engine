#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"

class Renderer{
    public:
        Renderer(const char* app_name);
        ~Renderer();
    public:
        bool RenderScene();
        void setWindow(GLFWwindow* window);
    private:
        bool checkValidationLayerSupport();
    private:
        bool isDebug = false;

        VkInstance m_instance;

        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface;

        VkQueue m_graphics_queue;
        VkQueue m_present_queue;

        VkDebugUtilsMessengerEXT m_debugMessenger;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
};

#endif
