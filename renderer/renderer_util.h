#ifndef RENDERER_UTIL
#define RENDERER_UTIL
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <limits>
#include <algorithm>
#include <optional>
#include <memory>
#include <set>
#include <map>
#include <unordered_map>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include "../error_handler/error_handler.h"

#include "vk_mem_alloc.h"

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
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

// _StructName or _ClassName means that a local object

struct alignas(16) _UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct _Object
{
    glm::mat4 pos;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferMemory;

    VkBuffer indexBuffer;
    VmaAllocation indexBufferMemory;

    VkBuffer uniformBuffer;
    VmaAllocation uniformBufferMemory;
    void *uniformBufferMapped;

    VkDescriptorSet *descriptorSet;
};

void createInstance(const char *appName, VkInstance *instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, const std::vector<const char *> &validationLayers, bool isDebug);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *outMessenger,
                         PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
void pickPhysicalDevice(VkInstance *instance, VkPhysicalDevice *physical_device, VkSurfaceKHR surface, bool isDebug);
void createLogicalDevice(VkQueue *graphicsQueue, VkQueue *presentQueue, VkDevice *device, VkPhysicalDevice *physical_device, const std::vector<const char *> &validationLayers, VkSurfaceKHR surface, bool isDebug);
void createSwapChain(VkDevice device, VkPhysicalDevice physical_device, GLFWwindow *window, VkSurfaceKHR surface, VkSwapchainKHR *swap_chain, std::vector<VkImage> &swap_chain_images, VkFormat *format, VkExtent2D *swap_chain_extent);
void createImageViews(std::vector<VkImageView> &swap_chain_image_views, std::vector<VkImage> &swap_chain_images, VkFormat format, VkDevice device);
void createGraphicsPipeline(const std::string vertex_shader_path, const std::string frag_shader_path, VkShaderModule *vert_shader_module, VkShaderModule *frag_shader_module, VkDescriptorSetLayout *descriptor_set_layout, std::vector<VkDynamicState> dynamic_states, VkViewport *viewport, VkRect2D *scissor, VkExtent2D extent, VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkPipeline *pipeline, VkDevice device);
void createRenderPass(VkRenderPass *render_pass, VkPipelineLayout *pipeline_layout, VkFormat swap_chain_image_format, VkDevice device);
void createFramebuffers(std::vector<VkFramebuffer> &framebuffers, std::vector<VkImageView> &swap_chain_image_views, VkRenderPass render_pass, VkExtent2D extent, VkDevice device);
void createCommandPool(VkCommandPool *command_pool, VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkDevice device);
void createCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int count, VkDevice device);
void createSecondaryCommandBuffers(std::vector<VkCommandBuffer> &command_buffers, VkCommandPool command_pool, int count, VkDevice device);
void createSyncObjects(std::vector<VkSemaphore> &imageAvailableSemaphores, std::vector<VkSemaphore> &renderFinishedSemaphores, std::vector<VkFence> &inFlightFences, int MAX_FRAMES_IN_FLIGHT, VkDevice device);
void recordCommandBuffer(VkCommandBuffer command_buffer, std::vector<ThreadData> &threads, std::unordered_map<std::string, std::unique_ptr<_Object>> &objects, uint32_t image_index, VkExtent2D extent, VkRenderPass render_pass, std::vector<VkFramebuffer> &framebuffers, VkPipeline graphics_pipeline, VkPipelineLayout pipeline_layout, uint32_t current_frame, glm::mat4 view, glm::mat4 proj, VkDevice device);
void recreateSwapChain(VkSwapchainKHR *swap_chain, VkRenderPass render_pass, std::vector<VkFramebuffer> &framebuffers, GLFWwindow *window, VkSurfaceKHR surface, std::vector<VkImage> &images, std::vector<VkImageView> &image_views, VkFormat *format, VkExtent2D *extent, VkPhysicalDevice physical_device, VkDevice device);
void createVertexBuffer(VkBuffer *vertex_buffer, std::vector<Vertex> vertices, VmaAllocation *vertex_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkPhysicalDevice physical_device, VkDevice device);
void createAllocator(VmaAllocator *allocator, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);
void createIndexBuffer(std::vector<uint16_t> indices, VkBuffer *index_buffer, VmaAllocation *index_buffer_memory, VkCommandPool command_pool, VkQueue graphics_queue, VmaAllocator allocator, VkDevice device);
void createDescriptorSetLayout(VkDescriptorSetLayout *descriptor_set_layout, VkDevice device);
void createUniformBuffer(VkBuffer *uniform_buffer, VmaAllocation *uniform_buffer_memory, void **uniform_buffer_mapped, VmaAllocator allocator);
void createDescriptorPool(VkDescriptorPool *descriptor_pool, uint32_t descriptor_count, VmaAllocator allocator, VkDevice device);
void addDescriptorSet(VkDescriptorSet descriptor_set, VkBuffer uniform_buffer, VkDevice device);
void createDescriptorSets(std::vector<VkDescriptorSet> &descriptor_sets, VkDescriptorSetLayout descriptor_set_layout, int count, VkDescriptorPool descriptor_pool, VkDevice device);
void createTextureImage(const char *texture_path, VkImage image, VmaAllocation image_memory, VmaAllocator allocator, VkCommandPool command_pool, VkQueue graphics_queue, VkDevice device);
static std::vector<char> readFile(const std::string &filename);
#endif