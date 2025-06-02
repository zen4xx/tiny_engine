#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <limits> 
#include <algorithm> 
#include <optional>
#include <set>
#include <map>

#include "../scene/scene.h"
#include "../error_handler/error_handler.h"

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void createInstance(const char* appName, VkInstance* instance ,PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, const std::vector<const char*>& validationLayers, bool isDebug);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* outMessenger,
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
void pickPhysicalDevice(VkInstance* instance, VkPhysicalDevice* physical_device, VkSurfaceKHR surface, bool isDebug);
void createLogicalDevice(VkQueue* graphicsQueue, VkQueue* presentQueue, VkDevice* device, VkPhysicalDevice* physical_device, const std::vector<const char*>& validationLayers, VkSurfaceKHR surface, bool isDebug);
void createSwapChain(VkDevice device, VkPhysicalDevice physical_device, GLFWwindow* window, VkSurfaceKHR surface, VkSwapchainKHR* swap_chain, std::vector<VkImage>* swap_chain_images, VkFormat* format, VkExtent2D* swap_chain_extent);
void createImageViews(std::vector<VkImageView> swap_chain_image_views, std::vector<VkImage> swap_chain_images, VkDevice device);
void createGraphicsPipeline(const std::string vertex_shader_path, const std::string frag_shader_path, VkShaderModule* vert_shader_module, VkShaderModule* frag_shader_module, std::vector<VkDynamicState> dynamic_states, VkViewport* viewport, VkRect2D* scissor, VkExtent2D extent, VkPipelineLayout* pipeline_layout, VkDevice device);
void createRenderPass(VkRenderPass* render_pass, VkPipelineLayout* pipeline_layout, VkFormat swap_chain_image_format, VkDevice device);
static std::vector<char> readFile(const std::string& filename);
