#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"

class Renderer{
    public:
        Renderer(const char* app_name);
        ~Renderer();
    public:
        bool RenderScene();
        void setWindow(GLFWwindow* window);
    private:
        bool checkValidationLayerSupport();
    private:
        bool isDebug = false;

        VkInstance m_instance;
        
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        
        VkSurfaceKHR m_surface;
        GLFWwindow* m_window;
        
        VkQueue m_graphics_queue;
        VkQueue m_present_queue;

        VkShaderModule m_vert_shader_module;
        VkShaderModule m_frag_shader_module;       

        VkDebugUtilsMessengerEXT m_debugMessenger;
        
        VkRenderPass m_render_pass;
        VkPipelineLayout m_pipeline_layout;

        VkViewport m_viewport;
        VkRect2D m_scissor;

        VkPipeline m_graphics_pipeline;

        VkSwapchainKHR m_swap_chain;
        std::vector<VkImage> m_swap_chain_images;
        VkFormat m_swap_chain_image_format;
        VkExtent2D m_swap_chain_extent;

        std::vector<VkImageView> m_swap_chain_image_views;
        std::vector<VkFramebuffer> m_swap_chain_frame_buffers;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        std::vector<VkDynamicState> m_dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
};

#endif
