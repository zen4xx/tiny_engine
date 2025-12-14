#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"
#include "../scene/scene.h"
#include <cstdint>

namespace tiny_engine
{
    struct Object
    {
        std::string scene_name;
        std::string obj_name;

        const std::vector<Vertex> *vertices = nullptr;
        const std::vector<uint32_t> *indices = nullptr;

        std::string gltf_model_path = "_null";

        std::string albedo_path = "_default";
        std::string mr_path = "_default";
        std::string normal_path = "_default";

        glm::mat4 pos;
    };    

};

class Renderer
{
public:
    Renderer(const char *app_name, bool is_debug = false);
    ~Renderer();

public:
    void setWindow(GLFWwindow *window);
    void drawScene(const std::string &scene_name);
    // Must be called before drawScene
    void addObject(const std::string &scene_name, const std::string &obj_name, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, glm::mat4 pos, const std::string &albedo_path = "_default", const std::string &mr_path = "_default", const std::string &normal_path = "_default");
    void addObject(const std::string &scene_name, const std::string &obj_name, const std::string &gltf_model_path, glm::mat4 pos, const std::string &albedo_path = "_default", const std::string &mr_path = "_default", const std::string &normal_path = "_default");
    void addObject(const tiny_engine::Object &obj);
    
    void deleteObject(const std::string &scene_name, const std::string &obj_name); 
    void deleteObject(const tiny_engine::Object &obj); 

    inline void moveObject(const std::string &scene_name, const std::string &obj_name, glm::mat4 pos) { m_scenes[scene_name]->objects[obj_name]->pc_data.model = pos; };
    inline void moveObject(const tiny_engine::Object &obj, glm::mat4 pos) { m_scenes[obj.scene_name]->objects[obj.obj_name]->pc_data.model = pos; };

    inline void setView(const std::string &scene_name, glm::mat4 view) { m_scenes[scene_name]->scene_data.view = view; };
    inline void setDrawDistance(const std::string &scene_name, float distance) { m_scenes[scene_name]->setDrawDistance(distance); };
    inline void setAmbient(const std::string &scene_name, glm::vec3 ambient) { m_scenes[scene_name]->scene_data.ambient = ambient; };
    inline void setDirLight(const std::string &scene_name, glm::vec3 dir) { m_scenes[scene_name]->scene_data.dirLight = dir; };
    inline void setDirLightColor(const std::string &scene_name, glm::vec3 color) { m_scenes[scene_name]->scene_data.dirLightColor = color; };
    inline void setPointLight(const std::string &scene_name, glm::vec3 pos, uint8_t index) { m_scenes[scene_name]->scene_data.pointLightPos[index] = glm::vec4(pos.x, pos.y, pos.z, 0.0f); };
    inline void setPointLightColor(const std::string &scene_name, glm::vec3 color, uint8_t index) { m_scenes[scene_name]->scene_data.pointLightColors[index] = glm::vec4(color.x, color.y, color.z, 0.0f); };
    inline void setPointLightsCount(const std::string &scene_name, int count) { m_scenes[scene_name]->scene_data.pointLightsCount = count; };

    // Must be called before setWindow
    inline void setThreadCount(uint8_t count) { m_thread_count = count; };
    inline void setMsaaQuality(uint8_t quality) { m_msaa_samples = static_cast<VkSampleCountFlagBits>(quality); };

    inline float getFPSCount() { return fps; };
    inline float getDeltaTime() { return m_delta_time; };

    inline void createScene(const std::string &scene_name) { m_scenes[scene_name] = std::move(std::make_unique<_Scene>(m_allocator)); };
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

    VkImage m_color_image;
    VmaAllocation m_color_image_memory;
    VkImageView m_color_image_view;

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
