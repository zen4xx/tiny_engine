#define STB_IMAGE_IMPLEMENTATION
#include "renderer.h"

/*INITIALIZATION VULKAN*/

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    std::cout << "[DEBUG] validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Renderer::Renderer(const char *app_name, bool is_debug)
{
    isDebug = is_debug;
    if (isDebug)
    {
        if (!checkValidationLayerSupport())
            err("Validation layers is requested, but not avaibled", 1);
    }

    createInstance(app_name, &m_instance, debugCallback, validationLayers, isDebug);
    if (isDebug)
        setupDebugMessenger(m_instance, &m_debugMessenger, debugCallback); // validation layers and debug output
}

Renderer::~Renderer()
{

    vkDeviceWaitIdle(m_device);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
        vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    for (auto thread : m_threads)
    {
        vkDestroyCommandPool(m_device, thread.command_pool, nullptr);
    }

    for (auto framebuffer : m_swap_chain_frame_buffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);

    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    for (auto imageView : m_swap_chain_image_views)
        vkDestroyImageView(m_device, imageView, nullptr);

    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it)
    {
        it->second->deleteScene(m_allocator, m_device);
    }
    vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);

    vkDestroyShaderModule(m_device, m_frag_shader_module, nullptr);
    vkDestroyShaderModule(m_device, m_vert_shader_module, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);

    if (m_debugMessenger)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(m_instance, m_debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(m_instance, nullptr);
}

bool Renderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProps : availableLayers)
        {
            if (strcmp(layerName, layerProps.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

/*ALSO INITIATE*/
void Renderer::setWindow(GLFWwindow *window)
{
    m_window = window;
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        err("Failed to create a surface", 1);

    pickPhysicalDevice(&m_instance, &m_physical_device, m_surface, isDebug);
    createLogicalDevice(&m_graphics_queue, &m_present_queue, &m_device, &m_physical_device, validationLayers, m_surface, isDebug);
    createAllocator(&m_allocator, m_instance, m_physical_device, m_device);
    createSwapChain(m_device, m_physical_device, m_window, m_surface, &m_swap_chain, m_swap_chain_images, &m_swap_chain_image_format, &m_swap_chain_extent);
    createImageViews(m_swap_chain_image_views, m_swap_chain_images, m_swap_chain_image_format, m_device);
    createRenderPass(&m_render_pass, &m_pipeline_layout, m_swap_chain_image_format, m_device);
    createDescriptorSetLayout(&m_descriptor_set_layout, m_device);
    createGraphicsPipeline("renderer/shaders/vert.spv", "renderer/shaders/frag.spv", &m_vert_shader_module, &m_frag_shader_module, &m_descriptor_set_layout, m_dynamic_states, &m_viewport, &m_scissor, m_swap_chain_extent, &m_render_pass, &m_pipeline_layout, &m_graphics_pipeline, m_device);
    createFramebuffers(m_swap_chain_frame_buffers, m_swap_chain_image_views, m_render_pass, m_swap_chain_extent, m_device);
    createCommandPool(&m_command_pool, m_surface, m_physical_device, m_device);
    createCommandBuffers(m_command_buffers, m_command_pool, MAX_FRAMES_IN_FLIGHT, m_device);

    m_threads.resize(m_thread_count);
    for (int i = 0; i < m_thread_count; ++i)
    {
        createCommandPool(&m_threads[i].command_pool, m_surface, m_physical_device, m_device);
        createSecondaryCommandBuffers(m_threads[i].command_buffers, m_threads[i].command_pool, MAX_FRAMES_IN_FLIGHT, m_device);
    }

    createSyncObjects(m_image_available_semaphores, m_render_finished_semaphores, m_in_flight_fences, MAX_FRAMES_IN_FLIGHT, m_device);
}

void Renderer::drawScene(const std::string &scene_name)
{

    if (!m_scenes[scene_name]->isCreated)
        m_scenes[scene_name]->createScene(m_swap_chain_extent, m_allocator, m_descriptor_set_layout, m_device);

    static double lastTime = glfwGetTime();

    double currentTime = glfwGetTime();
    m_delta_time = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;

    fps = m_delta_time > 0.0f ? 1.0f / m_delta_time : 0.0f;

    vkWaitForFences(m_device, 1, &m_in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, m_image_available_semaphores[current_frame], VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(&m_swap_chain, m_render_pass, m_swap_chain_frame_buffers, m_window, m_surface, m_swap_chain_images, m_swap_chain_image_views, &m_swap_chain_image_format, &m_swap_chain_extent, m_physical_device, m_device);
        return;
    }

    vkResetFences(m_device, 1, &m_in_flight_fences[current_frame]);
    vkResetCommandBuffer(m_command_buffers[current_frame], 0);
    recordCommandBuffer(m_command_buffers[current_frame], m_threads, m_scenes[scene_name]->objects, imageIndex, m_swap_chain_extent, m_render_pass, m_swap_chain_frame_buffers, m_graphics_pipeline, m_pipeline_layout, current_frame, m_scenes[scene_name]->view, m_scenes[scene_name]->proj, m_device);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_image_available_semaphores[current_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_command_buffers[current_frame];

    VkSemaphore signalSemaphore[] = {m_render_finished_semaphores[current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphore;

    vkQueueSubmit(m_graphics_queue, 1, &submitInfo, m_in_flight_fences[current_frame]);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphore;

    VkSwapchainKHR swapChains[] = {m_swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    res = vkQueuePresentKHR(m_present_queue, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        recreateSwapChain(&m_swap_chain, m_render_pass, m_swap_chain_frame_buffers, m_window, m_surface, m_swap_chain_images, m_swap_chain_image_views, &m_swap_chain_image_format, &m_swap_chain_extent, m_physical_device, m_device);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::addObject(std::string scene_name, std::string name, std::vector<Vertex> vertices, std::vector<uint16_t> indices, glm::mat4 pos)
{
    auto object = std::make_unique<_Object>();
    object->vertices = vertices;
    object->indices = indices;
    object->pos = pos;

    createVertexBuffer(&object->vertexBuffer, object->vertices, &object->vertexBufferMemory, m_command_pool, m_graphics_queue, m_allocator, m_physical_device, m_device);
    createIndexBuffer(object->indices, &object->indexBuffer, &object->indexBufferMemory, m_command_pool, m_graphics_queue, m_allocator, m_device);
    createUniformBuffer(&object->uniformBuffer, &object->uniformBufferMemory, &object->uniformBufferMapped, m_allocator);

    // moves object to scene
    m_scenes[scene_name]->objects[name] = std::move(object);
}

void Renderer::createScene(std::string name)
{
    auto scene = std::make_unique<_Scene>();
    m_scenes[name] = std::move(scene);
}