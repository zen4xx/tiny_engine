#ifndef RENDERER_H
#define RENDERER_H

#include "../scene/scene.h"
#include "../error_handler/error_handler.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>

//including vulkan
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete(){
        return graphicsFamily.has_value();
    }
};

class Renderer{
    public:
        Renderer(const char* app_name);
        ~Renderer();
    public:
        bool RenderScene();
    private:
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void pickPhysicalDevice();
        void createLogicalDevice();
    private:
        VkApplicationInfo m_app_info{};
        VkInstanceCreateInfo m_create_info{};

        VkDeviceQueueCreateInfo m_queue_create_info{};
        VkDeviceCreateInfo m_logical_create_info{};

        VkInstance m_instance;
        VkResult m_result;

        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device;

        VkSurfaceKHR m_surface;

        VkQueue m_graphics_queue;

        VkDebugUtilsMessengerEXT m_debugMessenger;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
};

#endif
