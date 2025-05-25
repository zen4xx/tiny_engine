#include "renderer_util.h"

std::vector<const char*> getRequiredExtensions(bool isDebug);

VkApplicationInfo app_info{};
VkInstanceCreateInfo create_info{};

/*CREATION AN INSTANCE*/
void createInstance(const char* appName, VkInstance* instance, const std::vector<const char*>& validationLayers, bool isDebug) {
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = appName;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "tiny engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfwExtCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    auto extensions = getRequiredExtensions(isDebug);
    for (uint32_t i = 0; i < glfwExtCount; ++i) {
        extensions.push_back(glfwExt[i]);
    }

    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    if (isDebug) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    VkResult res = vkCreateInstance(&create_info, nullptr, instance);
    if (res != VK_SUCCESS)
        err("Failed to create instance");
}

/*VALIDATION LAYERS*/
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* outMessenger,
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {

    if (!outMessenger || !debugCallback) return;

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

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        if (func(instance, &createInfo, nullptr, outMessenger) != VK_SUCCESS) {
            err("Failed to set up debug messenger!");
        }
    } 
    else {
        err("Could not load vkCreateDebugUtilsMessengerEXT");
    }
}

/*UTILS*/
std::vector<const char*> getRequiredExtensions(bool isDebug) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (isDebug) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    return extensions;
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

/*PICK PHYSICAL DEVICE*/
void pickPhysicalDevice(VkInstance* instance, VkPhysicalDevice* physical_device, bool isDebug){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

    if(deviceCount == 0){
        err("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

    //ordered map automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& device : devices){
        int score = rateDeviceSuitablity(device);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first > 0){
        *physical_device = candidates.rbegin()->second;
    }
    else err("Failed to find suitable GPU");

    if(isDebug){
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(*physical_device, &deviceProperties);
        std::cout << "picked GPU: " << deviceProperties.deviceName << std::endl;
    }
}

/*CREATION LOGICAL DEVICE*/
void createLogicalDevice(VkQueue* graphicsQueue, VkDevice* device, VkPhysicalDevice* physical_device, const std::vector<const char*>& validationLayers, bool isDebug){
    QueueFamilyIndices indices = findQueueFamilies(*physical_device);

    VkDeviceQueueCreateInfo queue_create_info{};
    VkDeviceCreateInfo logical_create_info{};

    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphicsFamily.value();
    queue_create_info.queueCount = 1;
    float queuePriority = 1.0f;
    queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    logical_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_create_info.pQueueCreateInfos = &queue_create_info;
    logical_create_info.queueCreateInfoCount = 1;

    logical_create_info.pEnabledFeatures = &deviceFeatures;
    logical_create_info.enabledExtensionCount = 0;

    if(isDebug){
        logical_create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        logical_create_info.ppEnabledLayerNames = validationLayers.data();
    }
    else
    logical_create_info.enabledLayerCount = 0;

    if(vkCreateDevice(*physical_device, &logical_create_info, nullptr, device) != VK_SUCCESS)
        err("Failed to create logical device");

    vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, graphicsQueue);
}
