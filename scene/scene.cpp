#include "scene.h"

void _Scene::createDescriptorSetsForScene(VkExtent2D extent, VmaAllocator allocator, VkDescriptorSetLayout descriptor_set_layout, VkDevice device)
{
    proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, draw_distance);

    createDescriptorPool(&descriptor_pool, static_cast<uint32_t>(objects.size()), allocator, device);
    createDescriptorSets(descriptor_sets, descriptor_set_layout, static_cast<uint32_t>(objects.size()), descriptor_pool, device);
    int i = 0;
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        addDescriptorSet(descriptor_sets[i], it->second->uniformBuffer, it->second->textureImageView, *it->second->sampler, device);
        it->second->descriptorSet = &descriptor_sets[i];
        ++i;
    }
    isDescriptorSetsCreated = 1;
}

void _Scene::destroyDescriptorPool(VkDevice device)
{
    if (descriptor_pool != VK_NULL_HANDLE)
    {
        descriptor_sets.clear();

        vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
        descriptor_pool = VK_NULL_HANDLE;
    }
}

void _Scene::deleteScene(VmaAllocator allocator, VkDevice device)
{
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        auto &obj = it->second;

        if (obj->uniformBufferMemory != VK_NULL_HANDLE && obj->uniformBufferMapped != nullptr)
        {
            vmaUnmapMemory(allocator, obj->uniformBufferMemory);
            obj->uniformBufferMapped = nullptr;
        }

        if (obj->vertexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, obj->vertexBuffer, obj->vertexBufferMemory);
        }

        if (obj->indexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, obj->indexBuffer, obj->indexBufferMemory);
        }

        if (obj->uniformBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, obj->uniformBuffer, obj->uniformBufferMemory);
        }
        
        if (obj->textureImage != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, obj->textureImageView, nullptr);
            vmaDestroyImage(allocator, obj->textureImage, obj->textureImageMemory);
        }
    }

    destroyDescriptorPool(device);

    isDeleted = 1;
}