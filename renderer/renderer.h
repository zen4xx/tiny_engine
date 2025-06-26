#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"

class Renderer
{
public:
    Renderer(const char *app_name, bool is_debug = false);
    ~Renderer();

public:
    void setWindow(GLFWwindow *window);
    void drawScene();
    // Must be called before drawScene
    void addObject(std::string name, std::vector<Vertex> vertices, std::vector<uint16_t> indices, glm::mat4 pos);
    inline void moveObject(std::string name, glm::mat4 pos) { m_objects[name]->pos = pos; };
    inline void setView(glm::mat4 view) { m_view = view; };
    // Must be called before setWindow
    inline void setThreadCount(uint8_t count) { m_thread_count = count; };
    inline float getFPSCount() { return fps; };
    inline float getDeltaTime() { return m_delta_time; };
    void createWorld();

private:
    bool checkValidationLayerSupport();

private:
    // booleans
    bool isDebug = false;
    bool isWorldCreated = false;

    VkInstance m_instance;

    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    VmaAllocator m_allocator;

    VkSurfaceKHR m_surface;
    GLFWwindow *m_window;

    VkQueue m_graphics_queue;
    VkQueue m_present_queue;

    VkShaderModule m_vert_shader_module;
    VkShaderModule m_frag_shader_module;

    VkDebugUtilsMessengerEXT m_debugMessenger;

    VkRenderPass m_render_pass;

    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkDescriptorPool m_descriptor_pool;

    VkDescriptorSetLayout m_descriptor_set_layout;
    VkPipelineLayout m_pipeline_layout;

    VkViewport m_viewport;
    VkRect2D m_scissor;

    VkPipeline m_graphics_pipeline;

    VkCommandPool m_command_pool;

    VkSwapchainKHR m_swap_chain;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;

    uint32_t current_frame = 0;

    // я хз как эта магия работает и если ты шакал захочешь это изменить то тебе п***а (8 вроде норм тк у меня еще многопоточка и получаеться что колчиество cmd buffers 8*numThread)
    const int MAX_FRAMES_IN_FLIGHT = 8;
    unsigned int m_thread_count = 6;

    float fps = 0;
    float m_delta_time = 0;

    std::unordered_map<std::string, std::unique_ptr<Object>> m_objects;
    glm::mat4 m_view = {0};
    glm::mat4 m_proj = {0};

    // vectors
    std::vector<VkImage> m_swap_chain_images;
    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<ThreadData> m_threads;

    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;

    std::vector<VkImageView> m_swap_chain_image_views;
    std::vector<VkFramebuffer> m_swap_chain_frame_buffers;
    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    std::vector<VkDynamicState> m_dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
};

#endif
