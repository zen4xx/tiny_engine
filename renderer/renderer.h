#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"
#include "../scene/scene.h"

class Renderer
{
public:
    Renderer(const char *app_name, bool is_debug = false);
    ~Renderer();

public:
    void setWindow(GLFWwindow *window);
    void drawScene(const std::string &scene_name);
    // Must be called before drawScene
    void addObject(std::string scene_name, std::string obj_name, std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::mat4 pos, std::string texture_path = "_default");
    void addObject(std::string scene_name, std::string obj_name, const std::string &gltf_model_path, glm::mat4 pos, std::string texture_path = "_default");

    inline void moveObject(const std::string &scene_name, const std::string &obj_name, glm::mat4 pos) { m_scenes[scene_name]->objects[obj_name]->pos = pos; };

    inline void setView(const std::string &scene_name, glm::mat4 view) { m_scenes[scene_name]->view = view; };
    inline void setDrawDistance(const std::string &scene_name, float distance) { m_scenes[scene_name]->setDrawDistance(distance); };
    // Must be called before setWindow
    inline void setThreadCount(uint8_t count) { m_thread_count = count; };
    inline float getFPSCount() { return fps; };
    inline float getDeltaTime() { return m_delta_time; };

    inline void createScene(const std::string &scene_name) { m_scenes[scene_name] = std::move(std::make_unique<_Scene>()); };
    inline void deleteScene(const std::string &scene_name)
    {
        m_scenes[scene_name]->deleteScene(m_allocator, m_device);
        m_scenes[scene_name]->isDeleted = 1;
    };
    // If objects are added to the scene, the function must be called before drawScene()
    inline void updateScene(const std::string &scene_name)
    {
        m_scenes[scene_name]->destroyDescriptorPool(m_device);
        m_scenes[scene_name]->createDescriptorSetsForScene(m_swap_chain_extent, m_allocator, m_descriptor_set_layout, m_device);
    };

private:
    bool checkValidationLayerSupport();

private:
    // booleans
    bool isDebug = false;

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

    VkDescriptorSetLayout m_descriptor_set_layout;
    VkPipelineLayout m_pipeline_layout;

    VkViewport m_viewport;
    VkRect2D m_scissor;

    VkPipeline m_graphics_pipeline;

    VkCommandPool m_command_pool;

    VkSwapchainKHR m_swap_chain;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;

    VkSampler m_sampler;

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage m_depth_image;
    VmaAllocation m_depth_image_memory;
    VkImageView m_depth_image_view;

    uint32_t current_frame = 0;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    unsigned int m_thread_count = 6;

    float fps = 0;
    float m_delta_time = 0;

    std::unordered_map<std::string, std::unique_ptr<_Scene>> m_scenes;

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
