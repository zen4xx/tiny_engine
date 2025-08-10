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
    _Scene(VmaAllocator allocator)
    {
        createUniformBuffer(&scene_data.uniformBuffer, &scene_data.uniformBufferMemory, &scene_data.uniformBufferMapped, allocator);
    }

public:
    void createDescriptorSetsForScene(VkExtent2D extent, VmaAllocator allocator, VkDescriptorSetLayout descriptor_set_layout, VkDevice device);

    void destroyDescriptorPool(VkDevice device);
    void deleteScene(VmaAllocator allocator, VkDevice device);

    inline void setDrawDistance(float distance) { draw_distance = distance; };

public:
    bool isDescriptorSetsCreated = 0;
    bool isDeleted = 0;

    std::unordered_map<std::string, std::unique_ptr<_Object>> objects;
    float draw_distance = 10;

    _SceneData scene_data;
};

#endif