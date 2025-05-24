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

int rateDeviceSuitablity(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

Renderer::Renderer(const char* app_name){

    #ifdef ENGINE_DEBUG
        if(!checkValidationLayerSupport()){
            err("Validation layers is requested, but not avaibled");
        }
    #endif
    /*CREATING AN INSTANCE*/
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
        err("Failed to create instance");
    }
    /*VALIDATION LAYERS*/
    #ifdef ENGINE_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
    
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            if (func(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
                err("Failed to set up debug messenger!");
            }
        } else {
            err("Could not load vkCreateDebugUtilsMessengerEXT");
        }
    #endif
    /*PICKING GPU*/
    pickPhysicalDevice();
    /*CREATION OF A LOGICAL DEVICE*/
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

std::vector<const char*> Renderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    #ifdef ENGINE_DEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

void Renderer::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(deviceCount == 0){
        err("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    //ordered map automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& device : devices){
        int score = rateDeviceSuitablity(device);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first > 0){
        m_physical_device = candidates.rbegin()->second;
    }
    else err("Failed to find suitable GPU");

    #ifdef ENGINE_DEBUG
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physical_device, &deviceProperties);
    std::cout << "picked GPU: " << deviceProperties.deviceName << std::endl;
    #endif
}

int rateDeviceSuitablity(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if(!deviceFeatures.geometryShader)
        return 0;
    
    QueueFamilyIndices indices = findQueueFamilies(device);
    if(!indices.isComplete())
        return 0;

    int score = 0;
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    
    score += deviceProperties.limits.maxImageDimension2D;
    
    return score;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        ++i;
    }

    return indices;
}

void Renderer::createLogicalDevice(){
    QueueFamilyIndices indices = findQueueFamilies(m_physical_device);

    m_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    m_queue_create_info.queueFamilyIndex = indices.graphicsFamily.value();
    m_queue_create_info.queueCount = 1;
    float queuePriority = 1.0f;
    m_queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    m_logical_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    m_logical_create_info.pQueueCreateInfos = &m_queue_create_info;
    m_logical_create_info.queueCreateInfoCount = 1;

    m_logical_create_info.pEnabledFeatures = &deviceFeatures;
    m_logical_create_info.enabledExtensionCount = 0;

    #ifdef ENGINE_DEBUG
    m_logical_create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    m_logical_create_info.ppEnabledLayerNames = validationLayers.data();
    #else
    m_logical_create_info.enabledLayerCount = 0;
    #endif

    if(vkCreateDevice(m_physical_device, &m_logical_create_info, nullptr, &m_device) != VK_SUCCESS)
        err("Failed to create logical device");

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphics_queue);
}
