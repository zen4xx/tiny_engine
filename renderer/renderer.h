#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"

class Renderer{
    public:
        Renderer(const char* app_name);
        ~Renderer();
    public:
        void setWindow(GLFWwindow* window);
        void drawScene();
        void addObject(std::vector<Vertex> vertices, std::vector<uint16_t> indices, glm::mat4 pos);
    private:
        bool checkValidationLayerSupport();
    private:
        bool isDebug = false;

        VkInstance m_instance;
        
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        
        VmaAllocator m_allocator;

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

        VkCommandPool m_command_pool;
        
        VkSwapchainKHR m_swap_chain;
        VkFormat m_swap_chain_image_format;
        VkExtent2D m_swap_chain_extent;
        
        uint32_t current_frame = 0;
        
        int MAX_FRAMES_IN_FLIGHT;
        
        //vectors
        std::vector<Object> m_objects;
        
        std::vector<VkImage> m_swap_chain_images;
        std::vector<VkCommandBuffer> m_command_buffers;

        std::vector<VkSemaphore> m_image_available_semaphores;
        std::vector<VkSemaphore> m_render_finished_semaphores;
        std::vector<VkFence> m_in_flight_fences;

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
