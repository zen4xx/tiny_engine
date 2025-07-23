#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "renderer_util.h"

std::vector<const char *> getRequiredExtensions(bool isDebug);

std::vector<const char *> required_extensions;
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME
};

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

/*CREATION AN INSTANCE*/
void createInstance(const char *appName, VkInstance *instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, const std::vector<const char *> &validationLayers, bool isDebug)
{
    required_extensions = getRequiredExtensions(isDebug);

    VkApplicationInfo appInfo{};
    VkInstanceCreateInfo createInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "tiny engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = required_extensions;

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (isDebug)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo, debugCallback);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult res = vkCreateInstance(&createInfo, nullptr, instance);
    if (res != VK_SUCCESS)
        err("Failed to create instance", res);
}
/*VMA*/
void createAllocator(VmaAllocator *allocator, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physical_device;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    vmaCreateAllocator(&allocatorInfo, allocator);
}

/*VALIDATION LAYERS*/
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *outMessenger,
                         PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{

    if (!outMessenger || !debugCallback)
        return;

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

    if (func != nullptr)
    {
        if (func(instance, &createInfo, nullptr, outMessenger) != VK_SUCCESS)
        {
            err("Failed to set up debug messenger!", 1);
        }
    }
    else
    {
        err("Could not load vkCreateDebugUtilsMessengerEXT", 1);
    }
}

/*UTILS*/
std::vector<const char *> getRequiredExtensions(bool isDebug)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (isDebug)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef _WIN32
    extensions.push_back("VK_KHR_win32_surface");
#elif __linux__
    if (std::getenv("WAYLAND_DISPLAY") != nullptr)
        extensions.push_back("VK_KHR_wayland_surface");
    else
        extensions.push_back("VK_KHR_xcb_surface");
#elif __APPLE__
    extensions.push_back("VK_MVK_macos_surface");
#endif

    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        ++i;
    }

    return indices;
}

uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    err("Failed to find suitable memory type", 0);
    return 0;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::optional<std::vector<std::string>> checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    if (extensionCount > 0)
    {
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    }

    std::set<std::string> requiredExtensions;
    for (const auto &ext : deviceExtensions)
    {
        requiredExtensions.emplace(ext);
    }

    for (const auto &prop : availableExtensions)
    {
        requiredExtensions.erase(prop.extensionName);
    }

    if (!requiredExtensions.empty())
    {
        std::vector<std::string> missing_extensions(requiredExtensions.begin(), requiredExtensions.end());
        return missing_extensions;
    }

    return std::nullopt;
}

int rateDeviceSuitablity(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (!deviceFeatures.geometryShader)
        return 0;

    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    auto missing = checkDeviceExtensionSupport(device);
    if (missing.has_value())
    {
        std::string msg = "Missing extensions:";
        for (const auto &ext : missing.value())
        {
            msg += " ";
            msg += ext;
        }
        err(msg.c_str(), 1);
    }

    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    if (!indices.isComplete() || !swapChainAdequate || !supportedFeatures.samplerAnisotropy)
        return 0;

    int score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

VkCommandBuffer beginSingleTimeCommands(VkCommandPool command_pool, VkDevice device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer command_buffer, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(command_pool, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, command_pool, graphics_queue, device);
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device){
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(command_pool, device);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    endSingleTimeCommands(commandBuffer, command_pool, graphics_queue, device);
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device){
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(command_pool, device);

    VkImageMemoryBarrier barrier{};
    
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) 
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        
    } 
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } 
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } 
    else
        throw std::invalid_argument("unsupported layout transition!");
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer, command_pool, graphics_queue, device);
}

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VmaAllocation &image_memory, VmaAllocator allocator)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = properties;

    VkResult res = vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &image_memory, nullptr);
    if (res != VK_SUCCESS)
        err("Failed to create image", res);
}

VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features))
            return format;
    }
    err("Failed to find supported format", 1);
    return VK_FORMAT_UNDEFINED;
}

VkFormat findDepthFormat(VkPhysicalDevice physical_device)
{
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physical_device);
}


/*PICK PHYSICAL DEVICE*/
void pickPhysicalDevice(VkInstance *instance, VkPhysicalDevice *physical_device, VkSurfaceKHR surface, bool isDebug)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        err("Failed to find GPUs with Vulkan support", 1);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

    // ordered map automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int score = rateDeviceSuitablity(device, surface);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        *physical_device = candidates.rbegin()->second;
    }
    else
        err("Failed to find suitable GPU", 1);

    if (isDebug)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(*physical_device, &deviceProperties);
        std::cout << "[DEBUG] picked GPU: " << deviceProperties.deviceName << std::endl;
    }
}

/*CREATION LOGICAL DEVICE*/
void createLogicalDevice(VkQueue *graphicsQueue, VkQueue *presentQueue, VkDevice *device, VkPhysicalDevice *physical_device, const std::vector<const char *> &validationLayers, VkSurfaceKHR surface, bool isDebug)
{
    QueueFamilyIndices indices = findQueueFamilies(*physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceNestedCommandBufferFeaturesEXT nestedCmdBufFeatures = {};
    nestedCmdBufFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT;
    nestedCmdBufFeatures.nestedCommandBuffer = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pNext = &nestedCmdBufFeatures;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (isDebug)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(*physical_device, &createInfo, nullptr, device) != VK_SUCCESS)
        err("Failed to create logical device", 1);

    vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, graphicsQueue);
    vkGetDeviceQueue(*device, indices.presentFamily.value(), 0, presentQueue);
}

/*CREATION OF SWAPCHAIN*/
void createSwapChain(VkDevice device, VkPhysicalDevice physical_device, GLFWwindow *window, VkSurfaceKHR surface, VkSwapchainKHR *swap_chain, std::vector<VkImage> &swap_chain_images, VkFormat *format, VkExtent2D *swap_chain_extent)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    *format = surfaceFormat.format;
    *swap_chain_extent = extent;

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(device, &createInfo, nullptr, swap_chain);
    if (res != VK_SUCCESS)
    {
        err("Failed to create swapchain", res);
    }

    vkGetSwapchainImagesKHR(device, *swap_chain, &imageCount, nullptr);
    swap_chain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, *swap_chain, &imageCount, swap_chain_images.data());

    *format = surfaceFormat.format;
    *swap_chain_extent = extent;
}

void createImageViews(std::vector<VkImageView> &swap_chain_image_views, std::vector<VkImage> &swap_chain_images, VkFormat format, VkDevice device)
{
    swap_chain_image_views.resize(swap_chain_images.size());
    if (swap_chain_image_views.empty())
        err("Swapchain images is empty", 0);
    for (size_t i = 0; i < swap_chain_images.size(); ++i)
    {

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swap_chain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView(device, &createInfo, nullptr, &swap_chain_image_views[i]);
        if (res != VK_SUCCESS)
            err("Failed to create image views", res);
    }
}

void createShaderModule(const std::vector<char> &code, VkShaderModule *shader_module, VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, shader_module);
    if (res != VK_SUCCESS)
        err("Failed to create shader module", res);
}

void createGraphicsPipeline(const std::string vertex_shader_path, const std::string frag_shader_path, VkShaderModule *vert_shader_module, VkShaderModule *frag_shader_module, VkDescriptorSetLayout *descriptor_set_layout, std::vector<VkDynamicState> dynamic_states, VkViewport *viewport, VkRect2D *scissor, VkExtent2D extent, VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkPipeline *pipeline, VkDevice device)
{
    auto vertShaderCode = readFile(vertex_shader_path);
    auto fragShaderCode = readFile(frag_shader_path);

    createShaderModule(vertShaderCode, vert_shader_module, device);
    createShaderModule(fragShaderCode, frag_shader_module, device);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = *vert_shader_module;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = *frag_shader_module;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamicState.pDynamicStates = dynamic_states.data();

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    viewport->x = 0.0f;
    viewport->y = 0.0f;
    viewport->width = (float)extent.width;
    viewport->height = (float)extent.height;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;

    scissor->offset = {0, 0};
    scissor->extent = extent;

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamicState.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptor_set_layout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; 
    depthStencil.maxDepthBounds = 1.0f; 
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {}; 

    VkResult res = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipeline_layout);
    if (res != VK_SUCCESS)
        err("Failed to create pipeline layout", res);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = *pipeline_layout;
    pipelineInfo.renderPass = *render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline);
    if (res != VK_SUCCESS)
        err("Failed to create graphics pipeline", res);
}
static std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        err((std::string("Failed to open file: ") + filename).c_str(), 0);

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void createRenderPass(VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkFormat swap_chain_image_format, VkPhysicalDevice physical_device, VkDevice device)
{
    VkAttachmentDescription colorAttachment{};

    colorAttachment.format = swap_chain_image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(physical_device);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult res = vkCreateRenderPass(device, &renderPassInfo, nullptr, render_pass);
    if (res != VK_SUCCESS)
        err("Failed to create render pass", res);
}

void createFramebuffers(std::vector<VkFramebuffer> &framebuffers, std::vector<VkImageView> &swap_chain_image_views, VkRenderPass render_pass, VkExtent2D extent, VkImageView depth_image_view, VkDevice device)
{
    framebuffers.resize(swap_chain_image_views.size());
    VkResult res;
    for (size_t i = 0; i < swap_chain_image_views.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {
            swap_chain_image_views[i],
            depth_image_view
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
        if (res != VK_SUCCESS)
        {
            err("Failed to create framebuffers", res);
        }
    }
}

void createCommandPool(VkCommandPool *command_pool, VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkDevice device)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physical_device, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult res = vkCreateCommandPool(device, &poolInfo, nullptr, command_pool);
    if (res != VK_SUCCESS)
        err("Failed to create command pool", res);
}

void createCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int MAX_FRAMES_IN_FLIGHT, VkDevice device)
{
    command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, command_buffers.data());
    if (res != VK_SUCCESS)
    {
        err("Failed to allocate command buffers", res);
    }
}

void createSecondaryCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int count, VkDevice device)
{
    command_buffers.resize(count);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = count;

    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, command_buffers.data());
    if (res != VK_SUCCESS)
    {
        err("Failed to allocate command buffers", res);
    }
}
void recordSecondary(ThreadData *thread, std::unordered_map<std::string, std::unique_ptr<_Object>> &objects, VkExtent2D extent, VkRenderPass render_pass, VkFramebuffer framebuffer, VkPipeline graphics_pipeline, VkPipelineLayout pipeline_layout, uint32_t current_frame, glm::mat4 view, glm::mat4 proj, VkDevice device, typename std::unordered_map<std::string, std::unique_ptr<_Object>>::const_iterator start, typename std::unordered_map<std::string, std::unique_ptr<_Object>>::const_iterator end)
{
    VkCommandBuffer cmd = thread->command_buffers[current_frame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = render_pass;
    inheritanceInfo.framebuffer = framebuffer;

    beginInfo.pInheritanceInfo = &inheritanceInfo;

    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &beginInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    _UniformBufferObject ubo;
    ubo.view = view;
    ubo.proj = proj;

    for (auto it = start; it != end; ++it)
    {
        ubo.model = it->second->pos;
        memcpy(it->second->uniformBufferMapped, &ubo, sizeof(ubo));

        VkBuffer vertexBuffers[] = {it->second->vertexBuffer};

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cmd, it->second->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, it->second->descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(it->second->indices.size()), 1, 0, 0, 0);
    }

    vkEndCommandBuffer(cmd);

    thread->is_cmd_buffer_recorded = 1;
}

void recordCommandBuffer(VkCommandBuffer command_buffer, std::vector<ThreadData> &threads, std::unordered_map<std::string, std::unique_ptr<_Object>> &objects, uint32_t image_index, VkExtent2D extent, VkRenderPass render_pass, std::vector<VkFramebuffer> &framebuffers, VkPipeline graphics_pipeline, VkPipelineLayout pipeline_layout, uint32_t current_frame, glm::mat4 view, glm::mat4 proj, VkDevice device)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    VkResult res = vkBeginCommandBuffer(command_buffer, &beginInfo);
    if (res != VK_SUCCESS)
        err("Failed to begin recording command buffer", res);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = framebuffers[image_index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    size_t numThreads = threads.size();

    size_t size = objects.size();
    size_t chunkSize = size / numThreads + 1;

    std::vector<std::thread> local_threads;

    auto it = objects.begin();
    for (int i = 0; i < numThreads && it != objects.end(); ++i) {
        auto start = it;
        for (size_t j = 0; j < chunkSize && it != objects.end(); ++j) {
            ++it;
        }
        local_threads.emplace_back(recordSecondary, &threads[i], std::ref(objects), extent, render_pass, framebuffers[image_index], graphics_pipeline, pipeline_layout, current_frame, view, proj, device, start, it);
    }

    int i = 0;
    for (auto& t : local_threads) {
        t.join();
        if(threads[i].is_cmd_buffer_recorded){
            vkCmdExecuteCommands(command_buffer, 1, &threads[i].command_buffers[current_frame]);
            threads[i].is_cmd_buffer_recorded = 0;
        }
        ++i;
    }

    vkCmdEndRenderPass(command_buffer);

    res = vkEndCommandBuffer(command_buffer);
    if (res != VK_SUCCESS)
        err("Failed to record command buffer", res);
}

void createSyncObjects(std::vector<VkSemaphore> &imageAvailableSemaphores, std::vector<VkSemaphore> &renderFinishedSemaphores, std::vector<VkFence> &inFlightFences, int MAX_FRAMES_IN_FLIGHT, VkDevice device)
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {

            err("Failed to create synchronization objects for a frame", 0);
        }
    }
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VmaAllocationCreateInfo *allocCreateInfo, VkBuffer *buffer, VmaAllocation *buffer_memory, VmaAllocator allocator)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo defaultAllocInfo{};
    if (!allocCreateInfo)
    {
        defaultAllocInfo.usage = memory_usage;
        defaultAllocInfo.flags =
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocCreateInfo = &defaultAllocInfo;
    }

    VmaAllocationInfo allocationInfo;
    VkResult res = vmaCreateBuffer(allocator, &bufferInfo, allocCreateInfo, buffer, buffer_memory, &allocationInfo);
    if (res != VK_SUCCESS)
    {
        err("Failed to create buffer with VMA", res);
    }
}

void cleanupSwapChain(std::vector<VkFramebuffer> &framebuffers, VkSwapchainKHR swap_chain, std::vector<VkImageView> &image_views, VkDevice device)
{
    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
    }

    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyImageView(device, image_views[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swap_chain, nullptr);
}

void recreateSwapChain(VkSwapchainKHR *swap_chain, VkRenderPass render_pass, std::vector<VkFramebuffer> &framebuffers, GLFWwindow *window, VkSurfaceKHR surface, std::vector<VkImage> &images, std::vector<VkImageView> &image_views, VkFormat *format, VkExtent2D *extent, VkImageView depth_image_view, VkPhysicalDevice physical_device, VkDevice device)
{
    cleanupSwapChain(framebuffers, *swap_chain, image_views, device);

    createSwapChain(device, physical_device, window, surface, swap_chain, images, format, extent);
    createImageViews(image_views, images, *format, device);
    createFramebuffers(framebuffers, image_views, render_pass, *extent, depth_image_view, device);
}

void createVertexBuffer(VkBuffer *vertex_buffer, std::vector<Vertex> vertices, VmaAllocation *vertex_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkPhysicalDevice physical_device, VkDevice device)
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 &allocCreateInfo,
                 &stagingBuffer,
                 &stagingBufferMemory,
                 allocator);

    void *data;
    vmaMapMemory(allocator, stagingBufferMemory, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(allocator, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 nullptr,
                 vertex_buffer,
                 vertex_buffer_memory,
                 allocator);

    copyBuffer(stagingBuffer, *vertex_buffer, bufferSize, command_pool, graphics_queue, device);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
}

void createIndexBuffer(std::vector<uint16_t> indices, VkBuffer *index_buffer, VmaAllocation *index_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkDevice device)
{
    VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, nullptr, &stagingBuffer, &stagingBufferMemory, allocator);

    void *data;
    vmaMapMemory(allocator, stagingBufferMemory, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vmaUnmapMemory(allocator, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, nullptr, index_buffer, index_buffer_memory, allocator);
    copyBuffer(stagingBuffer, *index_buffer, bufferSize, command_pool, graphics_queue, device);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
}

void createDescriptorSetLayout(VkDescriptorSetLayout *descriptor_set_layout, VkDevice device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, descriptor_set_layout);
    if (res != VK_SUCCESS)
        err("Failed to create descriptor set layout", res);
}

void createUniformBuffer(VkBuffer *uniform_buffer, VmaAllocation *uniform_buffer_memory, void **uniform_buffer_mapped, VmaAllocator allocator)
{
    VkDeviceSize bufferSize = sizeof(_UniformBufferObject);
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, nullptr, uniform_buffer, uniform_buffer_memory, allocator);
    vmaMapMemory(allocator, *uniform_buffer_memory, uniform_buffer_mapped);
}

void createDescriptorPool(VkDescriptorPool *descriptor_pool, uint32_t descriptor_count, VmaAllocator allocator, VkDevice device)
{
    
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = descriptor_count;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = descriptor_count;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = descriptor_count;

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, allocator->GetAllocationCallbacks(), descriptor_pool);
    if (res != VK_SUCCESS)
        err("Failed to create descriptor pool", res);
}

void createDescriptorSets(std::vector<VkDescriptorSet> &descriptor_sets, VkDescriptorSetLayout descriptor_set_layout, int count, VkDescriptorPool descriptor_pool, VkDevice device)
{
    std::vector<VkDescriptorSetLayout> layouts(count, descriptor_set_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
    allocInfo.pSetLayouts = layouts.data();

    descriptor_sets.resize(count);
    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, descriptor_sets.data());
    if (res != VK_SUCCESS)
        err("Failed to allocate descriptor sets", res);
}

void addDescriptorSet(VkDescriptorSet descriptor_set, VkBuffer uniform_buffer, VkImageView texture_image_view, VkSampler texture_sampler, VkDevice device)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniform_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(_UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture_image_view;
    imageInfo.sampler = texture_sampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor_set;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor_set;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void createTextureImage(const char *texture_path, VkImage &image, VmaAllocation &image_memory, VmaAllocator allocator, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device)
{
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(texture_path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 is channels count (rgba)

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, nullptr, &stagingBuffer, &stagingBufferMemory, allocator);
    
    void *data;
    vmaMapMemory(allocator, stagingBufferMemory, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory, allocator);

    transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, command_pool, graphics_queue, device);
    copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), command_pool, graphics_queue, device);

    transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, command_pool, graphics_queue, device);
    
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
}

void createTextureImageView(VkImageView *texture_image_view, VkImage image, VkDevice device)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(device, &viewInfo, nullptr, texture_image_view);
    if (res != VK_SUCCESS)
    {
        err("Failed to create texture image view", res);
    }
}

void createTextureSampler(VkSampler *texture_sampler, VkPhysicalDevice physical_device, VkDevice device){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult res = vkCreateSampler(device, &samplerInfo, nullptr, texture_sampler);
    if (res != VK_SUCCESS)
        err("Failed to create texture sampler", res);
}

void createDepthResources(VkImage &depth_image, VmaAllocation &depth_image_memory, VkImageView &depth_image_view, VmaAllocator allocator, VkExtent2D swap_chain_extent, VkQueue graphics_queue, VkCommandPool command_pool, VkPhysicalDevice physical_device, VkDevice device)
{
    VkFormat depthFormat = findDepthFormat(physical_device);
    
    createImage(swap_chain_extent.width, swap_chain_extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory, allocator);
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depth_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &depth_image_view);
    if (res != VK_SUCCESS)
    {
        err("Failed to create depth image view", res);
    }
    
    transitionImageLayout(depth_image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, command_pool, graphics_queue, device);
}

bool loadModel(const std::string& filename, _Object& object)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err_msg;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err_msg, &warn, filename);
    if (!ret) {
        std::string err_msg = "Failed to load glTF model: " + err_msg;
        err(err_msg.c_str(), 1);
        return false;
    }

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {

            const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& positionView = model.bufferViews[positionAccessor.bufferView];
            const tinygltf::Buffer& positionBuffer = model.buffers[positionView.buffer];

            const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const tinygltf::BufferView& texCoordView = model.bufferViews[texCoordAccessor.bufferView];
            const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordView.buffer];

            for (size_t i = 0; i < positionAccessor.count; ++i) {
                Vertex vertex;

                // Позиция
                vertex.pos.x = positionBuffer.data[positionView.byteOffset + positionAccessor.byteOffset + i * sizeof(float) * 3 + 0 * sizeof(float)];
                vertex.pos.y = positionBuffer.data[positionView.byteOffset + positionAccessor.byteOffset + i * sizeof(float) * 3 + 1 * sizeof(float)];
                vertex.pos.z = positionBuffer.data[positionView.byteOffset + positionAccessor.byteOffset + i * sizeof(float) * 3 + 2 * sizeof(float)];

                vertex.texCoord.x = texCoordBuffer.data[texCoordView.byteOffset + texCoordAccessor.byteOffset + i * sizeof(float) * 2 + 0 * sizeof(float)];
                vertex.texCoord.y = texCoordBuffer.data[texCoordView.byteOffset + texCoordAccessor.byteOffset + i * sizeof(float) * 2 + 1 * sizeof(float)];

                object.vertices.push_back(vertex);
            }

            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = model.buffers[indexView.buffer];

            for (size_t i = 0; i < indexAccessor.count; ++i) {
                uint16_t index = *(uint16_t*)(indexBuffer.data.data() + indexView.byteOffset + indexAccessor.byteOffset + i * sizeof(uint16_t));
                object.indices.push_back(index);
            }
        }
    }

    return true;
}
