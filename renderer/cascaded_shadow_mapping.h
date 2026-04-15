#ifndef CASCASED_SHADOW_MAPPING_H
#define CASCASED_SHADOW_MAPPING_H

#include "../renderer/renderer_util.h"
#include <array>

namespace tiny_engine
{
    // Number of cascades - typically 3-4 for good quality/performance balance
    constexpr uint32_t CASCADE_COUNT = 4;
    
    // Shadow map resolution per cascade
    constexpr uint32_t SHADOW_MAP_RESOLUTION = 2048;
    
    // Cascade split ratios (lambda method)
    constexpr float CASCADE_SPLIT_LAMBDA = 0.5f;
    
    // Shadow bias to prevent shadow acne
    constexpr float SHADOW_BIAS = 0.005f;
    
    // Normal offset bias for better slope handling
    constexpr float SHADOW_NORMAL_BIAS = 0.0f;
    
    // Percentage Closer Filtering (PCF) kernel size
    constexpr uint32_t PCF_KERNEL_SIZE = 3;
    
    struct CascadeData
    {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 lightSpaceMatrix; // projection * view
        
        // Frustum bounds in camera space for this cascade
        glm::vec3 frustumNearCorner[8];
        glm::vec3 frustumFarCorner[8];
        
        float nearPlane;
        float farPlane;
    };
    
    struct ShadowUniformBufferObject
    {
        alignas(16) glm::mat4 lightSpaceMatrices[CASCADE_COUNT];
        alignas(16) glm::vec4 cascadeSplits; // XYZW = near/far distances for each cascade
        alignas(4) float shadowBias;
        alignas(4) float normalBias;
    };
    
    struct CascadedShadowMap
    {
        VkImage shadowImages[CASCADE_COUNT];
        VmaAllocation shadowImageAllocations[CASCADE_COUNT];
        VkImageView shadowImageViews[CASCADE_COUNT];
        VkFramebuffer shadowFramebuffers[CASCADE_COUNT];
        
        VkRenderPass shadowRenderPass;
        VkPipelineLayout shadowPipelineLayout;
        VkPipeline shadowPipeline;
        VkDescriptorSetLayout shadowDescriptorSetLayout;
        VkDescriptorPool shadowDescriptorPool;
        VkDescriptorSet shadowDescriptorSets[CASCADE_COUNT];
        
        VkBuffer shadowUniformBuffer;
        VmaAllocation shadowUniformBufferMemory;
        void* shadowUniformBufferMapped;
        
        bool isInitialized = false;
    };
}

// Forward declarations for implementation
class Renderer;

namespace csm
{
    // Initialize the cascaded shadow mapping system
    void initCascadedShadowMaps(
        tiny_engine::CascadedShadowMap& csm,
        VmaAllocator allocator,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue graphicsQueue,
        VkCommandPool commandPool,
        uint32_t shadowMapResolution = tiny_engine::SHADOW_MAP_RESOLUTION,
        uint32_t cascadeCount = tiny_engine::CASCADE_COUNT
    );
    
    // Cleanup resources
    void cleanupCascadedShadowMaps(
        tiny_engine::CascadedShadowMap& csm,
        VkDevice device,
        VmaAllocator allocator
    );
    
    // Calculate cascade splits based on view frustum
    void calculateCascadeSplits(
        tiny_engine::CascadeData cascades[tiny_engine::CASCADE_COUNT],
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        float nearPlane,
        float farPlane,
        glm::vec3 lightDirection
    );
    
    // Update shadow matrices for current frame
    void updateShadowMatrices(
        tiny_engine::CascadedShadowMap& csm,
        tiny_engine::CascadeData cascades[tiny_engine::CASCADE_COUNT],
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        glm::vec3 lightDirection,
        float nearPlane,
        float farPlane
    );
    
    // Record commands to render shadow maps
    void recordShadowMapCommands(
        VkCommandBuffer commandBuffer,
        const tiny_engine::CascadedShadowMap& csm,
        const std::unordered_map<std::string, std::unique_ptr<_Object>>& objects,
        uint32_t currentFrame,
        VkDevice device
    );
    
    // Create shadow sampler for sampling in fragment shader
    void createShadowSampler(
        VkSampler* shadowSampler,
        VkPhysicalDevice physicalDevice,
        VkDevice device
    );
}

#endif // CASCASED_SHADOW_MAPPING_H
