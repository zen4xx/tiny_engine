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

#include "../scene/scene.h"
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

struct ThreadData{
    std::vector<VkCommandBuffer> command_buffers;
    VkCommandPool command_pool;

    bool is_cmd_buffer_recorded;
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
static std::vector<char> readFile(const std::string &filename);
