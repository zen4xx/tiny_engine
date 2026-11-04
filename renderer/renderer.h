#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_util.h"
#include "../scene/scene.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace tiny_engine
{
    struct Object
    {
        size_t scene_id;
        size_t obj_id;

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
    void drawScene(size_t scene_id);
    // Must be called before drawScene
    // Returns object_id
    size_t addObject(size_t scene_id, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, glm::mat4 pos, const std::string &albedo_path = "_default", const std::string &mr_path = "_default", const std::string &normal_path = "_default");
    size_t addObject(size_t scene_id, const std::string &gltf_model_path, glm::mat4 pos, const std::string &albedo_path = "_default", const std::string &mr_path = "_default", const std::string &normal_path = "_default");
    size_t addObject(tiny_engine::Object &obj);
    
    void deleteObject(size_t scene_id, size_t obj_id); 
    void deleteObject(const tiny_engine::Object &obj); 

    inline void moveObject(size_t scene_id, size_t obj_id, glm::mat4 pos) { m_scenes[scene_id].objects[obj_id].pc_data.model = pos; };
    inline void moveObject(const tiny_engine::Object &obj, glm::mat4 pos) { m_scenes[obj.scene_id].objects[obj.obj_id].pc_data.model = pos; };

    inline void setView(size_t scene_id, glm::mat4 view) { m_scenes[scene_id].scene_data.view = view; };
    inline void setDrawDistance(size_t scene_id, float distance) { m_scenes[scene_id].setDrawDistance(distance); };
    inline void setAmbient(size_t scene_id, glm::vec3 ambient) { m_scenes[scene_id].scene_data.ambient = ambient; };
    inline void setDirLight(size_t scene_id, glm::vec3 dir) { m_scenes[scene_id].scene_data.dirLight = dir; };
    inline void setDirLightColor(size_t scene_id, glm::vec3 color) { m_scenes[scene_id].scene_data.dirLightColor = color; };
    inline void setPointLight(size_t scene_id, glm::vec3 pos, uint8_t index) { m_scenes[scene_id].scene_data.pointLightPos[index] = glm::vec4(pos.x, pos.y, pos.z, 0.0f); };
    inline void setPointLightColor(size_t scene_id, glm::vec3 color, uint8_t index) { m_scenes[scene_id].scene_data.pointLightColors[index] = glm::vec4(color.x, color.y, color.z, 0.0f); };
    inline void setPointLightsCount(size_t scene_id, int count) { m_scenes[scene_id].scene_data.pointLightsCount = count; };

    // Must be called before setWindow
    inline void setThreadCount(uint8_t count) { m_thread_count = count; };
    inline void setMsaaQuality(uint8_t quality) { m_msaa_samples = static_cast<VkSampleCountFlagBits>(quality); };

    inline float getFPSCount() { return fps; };
    inline float getDeltaTime() { return m_delta_time; };

    // returns scene id
    inline size_t createScene(const std::string &skybox_path) { 
        _Scene scene(m_allocator);
        m_scenes.push_back(std::move(scene));
        size_t scene_id = m_scenes.size() - 1;

        createTextureSampler(&m_scenes[scene_id].scene_data.skyboxSampler, m_physical_device, m_device, 9);
        if (skybox_path != "_default")
            createTextureImage(skybox_path.c_str(), m_scenes[scene_id].scene_data.skyboxImage, m_scenes[scene_id].scene_data.skyboxMemory, m_allocator, m_command_pool, m_graphics_queue, m_device, 9, m_physical_device);
        else
            createTextureImage("tiny_engine_assets/textures/skybox.hdr", m_scenes[scene_id].scene_data.skyboxImage, m_scenes[scene_id].scene_data.skyboxMemory, m_allocator, m_command_pool, m_graphics_queue, m_device, 9, m_physical_device);
    
        createTextureImageView(&m_scenes[scene_id].scene_data.skyboxView, m_scenes[scene_id].scene_data.skyboxImage, m_device, 9);

        return scene_id;
    };
    inline void deleteScene(size_t scene_id)
    {
        m_scenes[scene_id].deleteScene(m_allocator, m_device);
        m_scenes[scene_id].isDeleted = 1;
    };
    // If objects are added to the scene, the function must be called before drawScene()
    inline void updateScene(size_t scene_id)
    {
        m_scenes[scene_id].destroyDescriptorPool(m_device);
        m_scenes[scene_id].createDescriptorSetsForScene(m_swap_chain_extent, m_allocator, m_descriptor_set_layout, m_shadow_image_view, m_shadow_sampler, m_device);
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

    VkShaderModule m_shadow_vert_shader_module;
    VkShaderModule m_shadow_frag_shader_module;

    VkImage m_shadow_image;
    VmaAllocation m_shadow_image_memory;
    VkImageView m_shadow_image_view;
    VkImageView m_shadow_layer_image_views[4];
    VkSampler m_shadow_sampler;

    VkRenderPass m_shadow_render_pass;
    std::vector<VkFramebuffer> m_shadow_framebuffers = {0, 0, 0, 0};
    VkPipelineLayout m_shadow_pipeline_layout;
    VkPipeline m_shadow_pipeline;

    uint32_t current_frame = 0;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    unsigned int m_thread_count = 6;

    float fps = 0;
    float m_delta_time = 0;

    std::vector<_Scene> m_scenes;

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
