#include "renderer_util.h"

std::vector<const char*> getRequiredExtensions(bool isDebug);

VkApplicationInfo app_info{};
VkInstanceCreateInfo create_info{};

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

/*CREATION AN INSTANCE*/
void createInstance(const char* appName, VkInstance* instance ,PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, const std::vector<const char*>& validationLayers, bool isDebug) {
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = appName;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "tiny engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto extensions = getRequiredExtensions(isDebug);

    #ifdef VK_USE_PLATFORM_MACOS_MOLTENVK
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (isDebug) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo, debugCallback);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
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

    #ifdef _WIN32
    extensions.push_back("VK_KHR_win32_surface");
    #elif __linux__
    if(std::getenv("WAYLAND_DISPLAY") != nullptr)
        extensions.push_back("VK_KHR_wayland_surface");
    else
        extensions.push_back("VK_KHR_xcb_surface"); 
    #elif __APPLE__
    extensions.push_back("VK_MVK_macos_surface");
    #endif


    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    return extensions;
}


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface){
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

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport){
            indices.presentFamily = i;
        }
        
        if (indices.isComplete()) {
            break;
        }
        
        ++i;
    }
    
    return indices;
}

int rateDeviceSuitablity(VkPhysicalDevice device, VkSurfaceKHR surface){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    if(!deviceFeatures.geometryShader)
    return 0;
    
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if(!indices.isComplete())
    return 0;
    
    int score = 0;
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    score += 1000;
    
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

/*PICK PHYSICAL DEVICE*/
void pickPhysicalDevice(VkInstance* instance, VkPhysicalDevice* physical_device, VkSurfaceKHR surface, bool isDebug){
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
        int score = rateDeviceSuitablity(device, surface);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first > 0){
        *physical_device = candidates.rbegin()->second;
    }
    else err("Failed to find suitable GPU");

    if(isDebug){
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(*physical_device, &deviceProperties);
        std::cout << "[DEBUG] picked GPU: " << deviceProperties.deviceName << std::endl;
    }
}

/*CREATION LOGICAL DEVICE*/
void createLogicalDevice(VkQueue* graphicsQueue, VkQueue* presentQueue, VkDevice* device, VkPhysicalDevice* physical_device, const std::vector<const char*>& validationLayers, VkSurfaceKHR surface, bool isDebug){
    QueueFamilyIndices indices = findQueueFamilies(*physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

    if(isDebug){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    if(vkCreateDevice(*physical_device, &createInfo, nullptr, device) != VK_SUCCESS)
        err("Failed to create logical device");

        vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, graphicsQueue);
        vkGetDeviceQueue(*device, indices.presentFamily.value(), 0, presentQueue);
}
