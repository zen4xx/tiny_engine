#include "renderer.h"
#define ENGINE_DEBUG

/*INITIALIZATION VULKAN*/

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cout << "[DEBUG] validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Renderer::Renderer(const char* app_name){

    #ifdef ENGINE_DEBUG
    isDebug = true;
    #endif
    
    if(isDebug){
        if(!checkValidationLayerSupport())
            err("Validation layers is requested, but not avaibled", 1);
    }

    createInstance(app_name, &m_instance, debugCallback, validationLayers, isDebug);
    if(isDebug)
        setupDebugMessenger(m_instance, &m_debugMessenger, debugCallback);//validation layers and debug output
}

Renderer::~Renderer(){
    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    for(auto framebuffer : m_swap_chain_frame_buffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    
    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    for(auto imageView : m_swap_chain_image_views)
        vkDestroyImageView(m_device, imageView, nullptr);
    
    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

    vkDestroyShaderModule(m_device, m_frag_shader_module, nullptr);
    vkDestroyShaderModule(m_device, m_vert_shader_module, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
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

/*ALSO INITIATE*/
void Renderer::setWindow(GLFWwindow* window){
    m_window = window;
    if(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        err("Failed to create a surface", 1);

    pickPhysicalDevice(&m_instance, &m_physical_device, m_surface, isDebug);
    createLogicalDevice(&m_graphics_queue, &m_present_queue, &m_device, &m_physical_device, validationLayers, m_surface, isDebug);
    createSwapChain(m_device, m_physical_device, m_window, m_surface, &m_swap_chain, &m_swap_chain_images, &m_swap_chain_image_format, &m_swap_chain_extent);
    createImageViews(m_swap_chain_image_views, m_swap_chain_images, m_device);
    createRenderPass(&m_render_pass, &m_pipeline_layout, m_swap_chain_image_format, m_device);
    createGraphicsPipeline("renderer/shaders/vert.spv", "renderer/shaders/frag.spv", &m_vert_shader_module, &m_frag_shader_module, m_dynamic_states, &m_viewport, &m_scissor, m_swap_chain_extent, &m_render_pass, &m_pipeline_layout, &m_graphics_pipeline, m_device);
    createFramebuffers(m_swap_chain_frame_buffers, m_swap_chain_image_views, m_render_pass, m_swap_chain_extent, m_device);
    createCommandPool(&m_command_pool, m_surface, m_physical_device, m_device);
    createCommandBuffer(&m_command_buffer, m_command_pool, m_device);
}