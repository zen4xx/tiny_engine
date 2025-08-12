#include "scene.h"
#include <vulkan/vulkan_core.h>

void _Scene::createDescriptorSetsForScene(VkExtent2D extent, VmaAllocator allocator, VkDescriptorSetLayout descriptor_set_layout, VkDevice device)
{
    scene_data.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, draw_distance);

    createDescriptorPool(&scene_data.descriptorPool, static_cast<uint32_t>(objects.size()), allocator, device);
    createDescriptorSets(scene_data.descriptorSets, descriptor_set_layout, static_cast<uint32_t>(objects.size()), scene_data.descriptorPool, device);
    int i = 0;
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        addDescriptorSet(scene_data.descriptorSets[i], scene_data.uniformBuffer, it->second->textureImageView, it->second->metalRoughnessImageView, it->second->normalImageView, *it->second->sampler, device);
        it->second->dc_index = i;
        ++i;
    }
    isDescriptorSetsCreated = 1;
}

void _Scene::destroyDescriptorPool(VkDevice device)
{
    if (scene_data.descriptorPool != VK_NULL_HANDLE)
    {
        scene_data.descriptorSets.clear();

        vkDestroyDescriptorPool(device, scene_data.descriptorPool, nullptr);
        scene_data.descriptorPool = VK_NULL_HANDLE;
    }
}

void _Scene::deleteScene(VmaAllocator allocator, VkDevice device)
{
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        auto &obj = it->second;
        
        if (obj->vertexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, obj->vertexBuffer, obj->vertexBufferMemory);
        }
        
        if (obj->indexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, obj->indexBuffer, obj->indexBufferMemory);
        }
        
        if (obj->textureImage != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, obj->textureImageView, nullptr);
            vmaDestroyImage(allocator, obj->textureImage, obj->textureImageMemory);
        }
        
        if (obj->normalImage != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, obj->normalImageView, nullptr);
            vmaDestroyImage(allocator, obj->normalImage, obj->normalImageMemory);
        }

        if (obj->metalRoughnessImage != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, obj->metalRoughnessImageView, nullptr);
            vmaDestroyImage(allocator, obj->metalRoughnessImage, obj->metalRoughnessImageMemory);
        }
    }

    if (scene_data.uniformBufferMemory != VK_NULL_HANDLE && scene_data.uniformBufferMapped != nullptr)
    {
        vmaUnmapMemory(allocator, scene_data.uniformBufferMemory);
        scene_data.uniformBufferMapped = nullptr;
    }
    
    if (scene_data.uniformBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(allocator, scene_data.uniformBuffer, scene_data.uniformBufferMemory);
    }

    destroyDescriptorPool(device);

    isDeleted = 1;
}