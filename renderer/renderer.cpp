#include "renderer.h"

//TODO: realise debug msg callback

Renderer::Renderer(const char* app_name){

    #ifdef ENGINE_DEBUG
        if(!checkValidationLayerSupport()){
            err("validation layers is requested, but not avaibled");
        }
    #endif

    m_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    m_app_info.pApplicationName = app_name;
    m_app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    m_app_info.pEngineName = "tiny engine";
    m_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    m_app_info.apiVersion = VK_API_VERSION_1_0;

    m_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    m_create_info.pApplicationInfo = &m_app_info;

    uint32_t glfwExtCount;
    const char** glfwExt;

    glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    m_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    auto extensions = getRequiredExtensions();
    m_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    m_create_info.ppEnabledExtensionNames = extensions.data();

    #ifdef ENGINE_DEBUG
        m_create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        m_create_info.ppEnabledLayerNames = validationLayers.data();
    #else
        m_create_info.enabledLayerCount = 0;
    #endif

    m_result = vkCreateInstance(&m_create_info, nullptr, &m_instance);

    if(m_result != VK_SUCCESS){
        err("failed to create instance");
    }
}

Renderer::~Renderer(){
    vkDestroyInstance(m_instance, nullptr);
}

bool Renderer::checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return false;
}

std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    #ifdef ENGINE_DEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    return extensions;
}