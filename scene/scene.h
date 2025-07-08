#ifndef SCENE_H
#define SCENE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <vector>
#include "vk_mem_alloc.h"

#include "../renderer/renderer_util.h"

class _Scene
{
public:
    void createScene(VkExtent2D extent, VmaAllocator allocator, VkDescriptorSetLayout descriptor_set_layout, VkDevice device)
    {
        proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
        createDescriptorPool(&descriptor_pool, objects.size(), allocator, device);
        createDescriptorSets(descriptor_sets, descriptor_set_layout, objects.size(), descriptor_pool, device);
        int i = 0;
        for (auto it = objects.begin(); it != objects.end(); ++it)
        {
            addDescriptorSet(descriptor_sets[i], it->second->uniformBuffer, device);
            it->second->descriptorSet = &descriptor_sets[i];
            ++i;
        }
        isCreated = 1;
    }

    void deleteScene(VmaAllocator allocator, VkDevice device)
    {
        for (auto it = objects.begin(); it != objects.end(); ++it)
        {
            vmaDestroyBuffer(allocator, it->second->vertexBuffer, it->second->vertexBufferMemory);
            vmaDestroyBuffer(allocator, it->second->indexBuffer, it->second->indexBufferMemory);
            vmaUnmapMemory(allocator, it->second->uniformBufferMemory);
            vmaDestroyBuffer(allocator, it->second->uniformBuffer, it->second->uniformBufferMemory);
        }

        vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    }

public:
    bool isCreated = 0;

    std::unordered_map<std::string, std::unique_ptr<_Object>> objects;
    glm::mat4 view = {0};
    glm::mat4 proj = {0};

    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;
};

#endif