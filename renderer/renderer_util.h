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

void createInstance(const char* appName, VkInstance* instance, const std::vector<const char*>& validationLayers, bool isDebug);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* outMessenger,
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
void pickPhysicalDevice(VkInstance* instance, VkPhysicalDevice* physical_device,  bool isDebug);
void createLogicalDevice(VkQueue* graphicsQueue, VkDevice* device, VkPhysicalDevice* physical_device, const std::vector<const char*>& validationLayers, bool isDebug);
