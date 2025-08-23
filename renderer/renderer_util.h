#ifndef RENDERER_UTIL
#define RENDERER_UTIL
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <memory>
#include <unordered_map>
#include "../include/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../include/tiny_gltf.h"
#include "vk_mem_alloc.h"

#define TINY_ENGINE_MAX_MSAA_QUALITY 255
#define MAX_POINT_LIGHTS_COUNT 16

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct ThreadData
{
    std::vector<VkCommandBuffer> command_buffers;
    VkCommandPool command_pool;

    bool is_cmd_buffer_recorded;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 tangent;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, tangent);

        return attributeDescriptions;
    }
};

// _StructName or _ClassName means that a local object

struct _UniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    
    alignas(16) glm::vec3 dirLight;
    alignas(16) glm::vec3 dirLightColor; 
    alignas(16) glm::vec3 ambient; 
    
    alignas(16) glm::vec4 point_light_colors[MAX_POINT_LIGHTS_COUNT]; // w for padding
    alignas(16) glm::vec4 point_light_pos[MAX_POINT_LIGHTS_COUNT]; // w for padding

    alignas(4) int point_lights_count;
};

struct _PushConstantsData
{ 
    glm::mat4 model;
};

struct _Object
{
    int dc_index; //descriptor set index 
    
    _PushConstantsData pc_data;
    
    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferMemory;
    
    VkBuffer indexBuffer;
    VmaAllocation indexBufferMemory;
    
    VkImage textureImage;
    VkImageView textureImageView;
    VmaAllocation textureImageMemory;

    VkImage metalRoughnessImage;
    VkImageView metalRoughnessImageView;
    VmaAllocation metalRoughnessImageMemory;

    VkImage normalImage;
    VkImageView normalImageView;
    VmaAllocation normalImageMemory;

    VkSampler *sampler;
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};


struct _SceneData
{
    glm::vec3 dirLight = {0.0f, 0.0f, 0.0f};
    glm::vec3 dirLightColor = {1.0f, 1.0f, 1.0f};
    glm::vec3 ambient = {0.1f, 0.1f, 0.1f};
    
    int pointLightsCount = 0;
    
    glm::vec4 pointLightColors[MAX_POINT_LIGHTS_COUNT] = {};
    glm::vec4 pointLightPos[MAX_POINT_LIGHTS_COUNT] = {};
    
    glm::mat4 view;
    glm::mat4 proj;
    
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    
    VkBuffer uniformBuffer;
    VmaAllocation uniformBufferMemory;
    void *uniformBufferMapped;
};

bool loadModel(const std::string &filename, _Object *object);

void createInstance(const char *appName, VkInstance *instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, const std::vector<const char *> &validationLayers, bool isDebug);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *outMessenger,
                         PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
void pickPhysicalDevice(VkInstance *instance, VkPhysicalDevice *physical_device, VkSurfaceKHR surface, bool isDebug);
void createLogicalDevice(VkQueue *graphicsQueue, VkQueue *presentQueue, VkDevice *device, VkPhysicalDevice *physical_device, const std::vector<const char *> &validationLayers, VkSurfaceKHR surface, bool isDebug);
void createSwapChain(VkDevice device, VkPhysicalDevice physical_device, GLFWwindow *window, VkSurfaceKHR surface, VkSwapchainKHR *swap_chain, std::vector<VkImage> &swap_chain_images, VkFormat *format, VkExtent2D *swap_chain_extent);
void createImageViews(std::vector<VkImageView> &swap_chain_image_views, std::vector<VkImage> &swap_chain_images, VkFormat format, VkDevice device);
void createGraphicsPipeline(const std::string vertex_shader_path, const std::string frag_shader_path, VkShaderModule *vert_shader_module, VkShaderModule *frag_shader_module, VkDescriptorSetLayout *descriptor_set_layout, std::vector<VkDynamicState> dynamic_states, VkViewport *viewport, VkRect2D *scissor, VkExtent2D extent, VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkPipeline *pipeline, VkSampleCountFlagBits msaa_samples, VkDevice device);
void createRenderPass(VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkFormat swap_chain_image_format, VkSampleCountFlagBits msaa_samples, VkPhysicalDevice physical_device, VkDevice device);
void createFramebuffers(std::vector<VkFramebuffer> &framebuffers, std::vector<VkImageView> &swap_chain_image_views, VkRenderPass render_pass, VkExtent2D extent, VkImageView color_image_view, VkImageView depth_image_view, VkDevice device);
void createCommandPool(VkCommandPool *command_pool, VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkDevice device);
void createCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int count, VkDevice device);
void createSecondaryCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int count, VkDevice device);
void createSyncObjects(std::vector<VkSemaphore> &imageAvailableSemaphores, std::vector<VkSemaphore> &renderFinishedSemaphores, std::vector<VkFence> &inFlightFences, int MAX_FRAMES_IN_FLIGHT, size_t swap_chain_image_count, VkDevice device);
void recordCommandBuffer(VkCommandBuffer command_buffer, std::vector<ThreadData> &threads, const std::unordered_map<std::string, std::unique_ptr<_Object>> &objects, uint32_t image_index, const VkExtent2D &extent, const VkRenderPass &render_pass, const std::vector<VkFramebuffer> &framebuffers, const VkPipeline &graphics_pipeline, const VkPipelineLayout &pipeline_layout, uint32_t current_frame, const _SceneData& scene_data, VkDevice device);
void recreateSwapChain(VkSwapchainKHR *swap_chain, VkRenderPass render_pass, std::vector<VkFramebuffer> &framebuffers, GLFWwindow *window, VkSurfaceKHR surface, std::vector<VkImage> &images, std::vector<VkImageView> &image_views, VkFormat *format, VkExtent2D *extent, VkImageView color_image_view, VkImageView depth_image_view, VkPhysicalDevice physical_device, VkDevice device);
void createVertexBuffer(VkBuffer *vertex_buffer, std::vector<Vertex> vertices, VmaAllocation *vertex_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkPhysicalDevice physical_device, VkDevice device);
void createAllocator(VmaAllocator *allocator, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);
void createIndexBuffer(std::vector<uint32_t> indices, VkBuffer *index_buffer, VmaAllocation *index_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkDevice device);
void createDescriptorSetLayout(VkDescriptorSetLayout *descriptor_set_layout, VkDevice device);
void createUniformBuffer(VkBuffer *uniform_buffer, VmaAllocation *uniform_buffer_memory, void **uniform_buffer_mapped, VmaAllocator allocator);
void createDescriptorPool(VkDescriptorPool *descriptor_pool, uint32_t descriptor_count, VmaAllocator allocator, VkDevice device);
void addDescriptorSet(VkDescriptorSet descriptor_set, VkBuffer uniform_buffer, VkImageView texture_image_view, VkImageView mr_image_view, VkImageView normal_image_view, VkSampler texture_sampler, VkDevice device);
void createDescriptorSets(std::vector<VkDescriptorSet> &descriptor_sets, VkDescriptorSetLayout descriptor_set_layout, uint32_t count, VkDescriptorPool descriptor_pool, VkDevice device);
void createTextureImage(const char *texture_path, VkImage &image, VmaAllocation &image_memory, VmaAllocator allocator, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device);
void createTextureImageView(VkImageView *texture_image_view, VkImage image, VkDevice device);
void createTextureSampler(VkSampler *texture_sampler, VkPhysicalDevice physical_device, VkDevice device);
void createDepthResources(VkImage &depth_image, VmaAllocation &depth_image_memory, VkImageView &depth_image_view, VmaAllocator allocator, VkExtent2D swap_chain_extent, VkQueue graphics_queue, VkCommandPool command_pool, VkSampleCountFlagBits msaa_samples, VkPhysicalDevice physical_device, VkDevice device);
void createColorResources(VkImage &color_image, VmaAllocation &color_image_memory, VkImageView &color_image_view, VmaAllocator allocator, VkExtent2D swap_chain_extent, VkFormat swap_chain_image_format, VkQueue graphics_queue, VkCommandPool command_pool, VkSampleCountFlagBits msaa_samples, VkDevice device);
void computeAABB(_Object& obj);

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physical_device);

static std::vector<char> readFile(const std::string &filename);
#endif