#include "cascaded_shadow_mapping.h"
#include "../error_handler/error_handler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <algorithm>
#include <cmath>

namespace csm
{
    // Helper function to create a single shadow map image
    void createShadowMapImage(
        VkImage& image,
        VmaAllocation& allocation,
        VkImageView& imageView,
        VmaAllocator allocator,
        uint32_t size,
        VkQueue graphicsQueue,
        VkCommandPool commandPool,
        VkPhysicalDevice physicalDevice,
        VkDevice device
    )
    {
        // Create image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = size;
        imageInfo.extent.height = size;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS)
            err("Failed to create shadow map image!", 1);

        // Create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            err("Failed to create shadow map image view!", 1);
    }

    void initCascadedShadowMaps(
        tiny_engine::CascadedShadowMap& csm,
        VmaAllocator allocator,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue graphicsQueue,
        VkCommandPool commandPool,
        uint32_t shadowMapResolution,
        uint32_t cascadeCount
    )
    {
        // Create shadow images for each cascade
        for (uint32_t i = 0; i < cascadeCount; ++i)
        {
            createShadowMapImage(
                csm.shadowImages[i],
                csm.shadowImageAllocations[i],
                csm.shadowImageViews[i],
                allocator,
                shadowMapResolution,
                graphicsQueue,
                commandPool,
                physicalDevice,
                device
            );
        }

        // Create render pass for shadow mapping
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &csm.shadowRenderPass) != VK_SUCCESS)
            err("Failed to create shadow render pass!", 1);

        // Create descriptor set layout
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &csm.shadowDescriptorSetLayout) != VK_SUCCESS)
            err("Failed to create shadow descriptor set layout!", 1);

        // Create pipeline layout
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &csm.shadowDescriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &csm.shadowPipelineLayout) != VK_SUCCESS)
            err("Failed to create shadow pipeline layout!", 1);

        // Create graphics pipeline (shaders must be compiled separately)
        // This is a placeholder - actual pipeline creation requires compiled SPIR-V shaders
        // The pipeline will be created in the renderer using shadow.vert.spv and shadow.frag.spv

        // Create descriptor pool
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = cascadeCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = cascadeCount;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &csm.shadowDescriptorPool) != VK_SUCCESS)
            err("Failed to create shadow descriptor pool!", 1);

        // Create descriptor sets
        std::vector<VkDescriptorSetLayout> layouts(cascadeCount, csm.shadowDescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = csm.shadowDescriptorPool;
        allocInfo.descriptorSetCount = cascadeCount;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(device, &allocInfo, csm.shadowDescriptorSets) != VK_SUCCESS)
            err("Failed to allocate shadow descriptor sets!", 1);

        // Create uniform buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(tiny_engine::ShadowUniformBufferObject);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &csm.shadowUniformBuffer, &csm.shadowUniformBufferMemory, nullptr) != VK_SUCCESS)
            err("Failed to create shadow uniform buffer!", 1);

        vmaMapMemory(allocator, csm.shadowUniformBufferMemory, &csm.shadowUniformBufferMapped);

        // Create framebuffers for each cascade
        for (uint32_t i = 0; i < cascadeCount; ++i)
        {
            VkImageView attachments[] = {csm.shadowImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = csm.shadowRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = shadowMapResolution;
            framebufferInfo.height = shadowMapResolution;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &csm.shadowFramebuffers[i]) != VK_SUCCESS)
                err("Failed to create shadow framebuffer!", 1);
        }

        csm.isInitialized = true;
    }

    void cleanupCascadedShadowMaps(
        tiny_engine::CascadedShadowMap& csm,
        VkDevice device,
        VmaAllocator allocator
    )
    {
        if (!csm.isInitialized)
            return;

        vkDeviceWaitIdle(device);

        for (uint32_t i = 0; i < tiny_engine::CASCADE_COUNT; ++i)
        {
            if (csm.shadowFramebuffers[i] != VK_NULL_HANDLE)
                vkDestroyFramebuffer(device, csm.shadowFramebuffers[i], nullptr);

            if (csm.shadowImageViews[i] != VK_NULL_HANDLE)
                vkDestroyImageView(device, csm.shadowImageViews[i], nullptr);

            if (csm.shadowImages[i] != VK_NULL_HANDLE)
                vmaDestroyImage(allocator, csm.shadowImages[i], csm.shadowImageAllocations[i]);
        }

        if (csm.shadowUniformBuffer != VK_NULL_HANDLE)
        {
            vmaUnmapMemory(allocator, csm.shadowUniformBufferMemory);
            vmaDestroyBuffer(allocator, csm.shadowUniformBuffer, csm.shadowUniformBufferMemory);
        }

        if (csm.shadowDescriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(device, csm.shadowDescriptorPool, nullptr);

        if (csm.shadowPipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(device, csm.shadowPipeline, nullptr);

        if (csm.shadowPipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device, csm.shadowPipelineLayout, nullptr);

        if (csm.shadowDescriptorSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device, csm.shadowDescriptorSetLayout, nullptr);

        if (csm.shadowRenderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(device, csm.shadowRenderPass, nullptr);

        csm.isInitialized = false;
    }

    // Get frustum corners in world space
    std::array<glm::vec3, 8> getFrustumCorners(const glm::mat4& invViewProj, float nearZ, float farZ)
    {
        std::array<glm::vec3, 8> corners;
        
        // NDC corners
        std::array<glm::vec4, 8> ndcCorners = {{
            {-1.0f, -1.0f, nearZ, 1.0f},
            { 1.0f, -1.0f, nearZ, 1.0f},
            { 1.0f,  1.0f, nearZ, 1.0f},
            {-1.0f,  1.0f, nearZ, 1.0f},
            {-1.0f, -1.0f, farZ, 1.0f},
            { 1.0f, -1.0f, farZ, 1.0f},
            { 1.0f,  1.0f, farZ, 1.0f},
            {-1.0f,  1.0f, farZ, 1.0f}
        }};

        for (size_t i = 0; i < 8; ++i)
        {
            glm::vec4 corner = invViewProj * ndcCorners[i];
            corner /= corner.w;
            corners[i] = glm::vec3(corner);
        }

        return corners;
    }

    void calculateCascadeSplits(
        tiny_engine::CascadeData cascades[tiny_engine::CASCADE_COUNT],
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        float nearPlane,
        float farPlane,
        glm::vec3 lightDirection
    )
    {
        glm::mat4 invViewProj = glm::inverse(projectionMatrix * viewMatrix);

        // Calculate cascade split points using logarithmic split scheme
        std::array<float, tiny_engine::CASCADE_COUNT + 1> cascadeSplits;
        cascadeSplits[0] = nearPlane;

        for (uint32_t i = 1; i <= tiny_engine::CASCADE_COUNT; ++i)
        {
            float p = static_cast<float>(i) / static_cast<float>(tiny_engine::CASCADE_COUNT);
            
            // Logarithmic split
            float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            // Uniform split
            float uniformSplit = nearPlane + (farPlane - nearPlane) * p;
            // Blend between logarithmic and uniform
            cascadeSplits[i] = logSplit * tiny_engine::CASCADE_SPLIT_LAMBDA + uniformSplit * (1.0f - tiny_engine::CASCADE_SPLIT_LAMBDA);
        }

        // Calculate matrices for each cascade
        for (uint32_t i = 0; i < tiny_engine::CASCADE_COUNT; ++i)
        {
            cascades[i].nearPlane = cascadeSplits[i];
            cascades[i].farPlane = cascadeSplits[i + 1];

            // Get frustum corners for this cascade
            auto nearCorners = getFrustumCorners(invViewProj, cascades[i].nearPlane, cascades[i].nearPlane);
            auto farCorners = getFrustumCorners(invViewProj, cascades[i].farPlane, cascades[i].farPlane);

            // Combine all corners
            std::array<glm::vec3, 8> allCorners;
            for (int j = 0; j < 4; ++j)
            {
                allCorners[j] = nearCorners[j];
                allCorners[j + 4] = farCorners[j];
            }

            // Find center of frustum slice
            glm::vec3 center(0.0f);
            for (const auto& corner : allCorners)
                center += corner;
            center /= 8.0f;

            // Create light view matrix (look at center from light direction)
            glm::vec3 lightPos = center - lightDirection;
            cascades[i].viewMatrix = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));

            // Transform corners to light space to find bounding box
            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            for (const auto& corner : allCorners)
            {
                glm::vec4 lightSpaceCorner = cascades[i].viewMatrix * glm::vec4(corner, 1.0f);
                minExtents = glm::min(minExtents, glm::vec3(lightSpaceCorner));
                maxExtents = glm::max(maxExtents, glm::vec3(lightSpaceCorner));
            }

            // Create orthographic projection for light
            float zNear = minExtents.z;
            float zFar = maxExtents.z;
            float xScale = (maxExtents.x - minExtents.x) * 0.5f;
            float yScale = (maxExtents.y - minExtents.y) * 0.5f;
            float centerX = (minExtents.x + maxExtents.x) * 0.5f;
            float centerY = (minExtents.y + maxExtents.y) * 0.5f;

            // Adjust center to account for light direction
            center -= glm::vec3(centerX, centerY, 0.0f);
            lightPos = center - lightDirection;
            cascades[i].viewMatrix = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));

            // Recalculate extents with adjusted center
            minExtents = glm::vec3(std::numeric_limits<float>::max());
            maxExtents = glm::vec3(std::numeric_limits<float>::lowest());

            for (const auto& corner : allCorners)
            {
                glm::vec4 lightSpaceCorner = cascades[i].viewMatrix * glm::vec4(corner, 1.0f);
                minExtents = glm::min(minExtents, glm::vec3(lightSpaceCorner));
                maxExtents = glm::max(maxExtents, glm::vec3(lightSpaceCorner));
            }

            cascades[i].projectionMatrix = glm::ortho(
                minExtents.x, maxExtents.x,
                minExtents.y, maxExtents.y,
                minExtents.z, maxExtents.z
            );

            cascades[i].lightSpaceMatrix = cascades[i].projectionMatrix * cascades[i].viewMatrix;
        }
    }

    void updateShadowMatrices(
        tiny_engine::CascadedShadowMap& csm,
        tiny_engine::CascadeData cascades[tiny_engine::CASCADE_COUNT],
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        glm::vec3 lightDirection,
        float nearPlane,
        float farPlane
    )
    {
        calculateCascadeSplits(cascades, viewMatrix, projectionMatrix, nearPlane, farPlane, lightDirection);

        // Update uniform buffer
        tiny_engine::ShadowUniformBufferObject* ubo = 
            reinterpret_cast<tiny_engine::ShadowUniformBufferObject*>(csm.shadowUniformBufferMapped);

        for (uint32_t i = 0; i < tiny_engine::CASCADE_COUNT; ++i)
        {
            ubo->lightSpaceMatrices[i] = cascades[i].lightSpaceMatrix;
        }

        ubo->cascadeSplits = glm::vec4(
            cascades[0].farPlane,
            cascades[1].farPlane,
            cascades[2].farPlane,
            cascades[3].farPlane
        );
        ubo->shadowBias = tiny_engine::SHADOW_BIAS;
        ubo->normalBias = tiny_engine::SHADOW_NORMAL_BIAS;
    }

    void recordShadowMapCommands(
        VkCommandBuffer commandBuffer,
        const tiny_engine::CascadedShadowMap& csm,
        const std::unordered_map<std::string, std::unique_ptr<_Object>>& objects,
        uint32_t currentFrame,
        VkDevice device
    )
    {
        for (uint32_t cascadeIdx = 0; cascadeIdx < tiny_engine::CASCADE_COUNT; ++cascadeIdx)
        {
            // Begin render pass for this cascade
            VkClearValue clearValue{};
            clearValue.depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = csm.shadowRenderPass;
            renderPassInfo.framebuffer = csm.shadowFramebuffers[cascadeIdx];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = {tiny_engine::SHADOW_MAP_RESOLUTION, tiny_engine::SHADOW_MAP_RESOLUTION};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, csm.shadowPipeline);
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                csm.shadowPipelineLayout,
                0,
                1,
                &csm.shadowDescriptorSets[cascadeIdx],
                0,
                nullptr
            );

            // Draw all objects
            for (const auto& [name, obj] : objects)
            {
                if (!obj || !obj->is_alive)
                    continue;

                VkBuffer vertexBuffers[] = {obj->vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(commandBuffer, obj->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                // Push model matrix as push constant
                vkCmdPushConstants(
                    commandBuffer,
                    csm.shadowPipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(glm::mat4),
                    &obj->pc_data.model
                );

                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(obj->indices->size()), 1, 0, 0, 0);
            }

            vkCmdEndRenderPass(commandBuffer);
        }
    }

    void createShadowSampler(
        VkSampler* shadowSampler,
        VkPhysicalDevice physicalDevice,
        VkDevice device
    )
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // Important for PCF
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_TRUE; // Enable comparison for shadow mapping
        samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, shadowSampler) != VK_SUCCESS)
            err("Failed to create shadow sampler!", 1);
    }
}
