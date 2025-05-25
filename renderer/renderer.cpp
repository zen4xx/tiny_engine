#include "renderer.h"
#define ENGINE_DEBUG

/*INITIALIZATION VULKAN*/

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Renderer::Renderer(const char* app_name){

    #ifdef ENGINE_DEBUG
    isDebug = true;
    #endif
    
    if(isDebug){
        if(!checkValidationLayerSupport())
            err("Validation layers is requested, but not avaibled");
    }

    createInstance(app_name, &m_instance, validationLayers, isDebug);
    if(isDebug)
        setupDebugMessenger(m_instance, &m_debugMessenger, debugCallback);//validation layers and debug output
    pickPhysicalDevice(&m_instance, &m_physical_device, isDebug);
    createLogicalDevice(&m_graphics_queue, &m_device, &m_physical_device, validationLayers, isDebug);
}

Renderer::~Renderer(){
    vkDestroyDevice(m_device, nullptr);
    if (m_debugMessenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_instance, m_debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(m_instance, nullptr);
}

bool Renderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProps : availableLayers) {
            if (strcmp(layerName, layerProps.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}
