# Cascaded Shadow Mapping (CSM) Implementation

This directory contains a complete implementation of Cascaded Shadow Mapping for the tiny_engine Vulkan renderer.

## Files Overview

### Header Files
- `cascaded_shadow_mapping.h` - Main CSM header with data structures and function declarations
- `renderer/cascaded_shadow_mapping.h` can be included in your renderer

### Source Files
- `cascaded_shadow_mapping.cpp` - Implementation of all CSM functionality

### Shader Files
- `shaders/shadow.vert` - Vertex shader for rendering shadow maps
- `shaders/shadow.frag` - Fragment shader for shadow map rendering
- `shaders/default_shader.frag` - Modified main fragment shader with CSM shadow sampling

## Key Features

1. **4 Cascade Splits** - Divides the view frustum into 4 regions for optimal shadow quality
2. **Logarithmic Split Scheme** - Blends logarithmic and uniform splits for balanced quality
3. **Percentage Closer Filtering (PCF)** - 3x3 kernel for soft shadow edges
4. **Configurable Parameters**:
   - `CASCADE_COUNT` - Number of cascades (default: 4)
   - `SHADOW_MAP_RESOLUTION` - Resolution per cascade (default: 2048)
   - `CASCADE_SPLIT_LAMBDA` - Blend factor for split scheme (default: 0.5)
   - `SHADOW_BIAS` - Depth bias to prevent shadow acne (default: 0.005)
   - `PCF_KERNEL_SIZE` - Filter kernel size (default: 3)

## Integration Steps

### 1. Add CSM to Renderer Class

```cpp
// In renderer.h
#include "cascaded_shadow_mapping.h"

class Renderer {
    // ... existing members ...
    
private:
    tiny_engine::CascadedShadowMap m_csm;
    tiny_engine::CascadeData m_cascades[tiny_engine::CASCADE_COUNT];
    VkSampler m_shadowSampler;
};
```

### 2. Initialize CSM

```cpp
// In renderer.cpp, after creating the device and command pool
void Renderer::setWindow(GLFWwindow *window)
{
    // ... existing initialization code ...
    
    // Initialize Cascaded Shadow Maps
    csm::initCascadedShadowMaps(
        m_csm,
        m_allocator,
        m_physical_device,
        m_device,
        m_graphics_queue,
        m_command_pool,
        tiny_engine::SHADOW_MAP_RESOLUTION,
        tiny_engine::CASCADE_COUNT
    );
    
    // Create shadow sampler
    csm::createShadowSampler(&m_shadowSampler, m_physical_device, m_device);
}
```

### 3. Update Shadow Matrices Each Frame

```cpp
// In drawScene(), before recording command buffers
void Renderer::drawScene(const std::string &scene_name)
{
    // ... existing code ...
    
    // Update CSM matrices
    glm::mat4 projection = m_scenes[scene_name]->scene_data.proj;
    glm::mat4 view = m_scenes[scene_name]->scene_data.view;
    glm::vec3 lightDir = normalize(m_scenes[scene_name]->scene_data.dirLight);
    
    csm::updateShadowMatrices(
        m_csm,
        m_cascades,
        view,
        projection,
        lightDir,
        0.1f,  // near plane
        100.0f // far plane
    );
}
```

### 4. Render Shadow Maps

```cpp
// Record shadow pass before main render pass
void recordShadowPass(VkCommandBuffer commandBuffer)
{
    csm::recordShadowMapCommands(
        commandBuffer,
        m_csm,
        m_scenes[scene_name]->objects,
        current_frame,
        m_device
    );
}
```

### 5. Compile Shaders

```bash
# Compile shadow shaders
glslc renderer/shaders/shadow.vert -o tiny_engine_assets/shaders/shadow.vert.spv
glslc renderer/shaders/shadow.frag -o tiny_engine_assets/shaders/shadow.frag.spv

# Recompile main fragment shader with CSM support
glslc renderer/shaders/default_shader.frag -o tiny_engine_assets/shaders/frag.spv
glslc renderer/shaders/default_shader.vert -o tiny_engine_assets/shaders/vert.spv
```

### 6. Create Shadow Pipeline

```cpp
// Create graphics pipeline for shadow rendering
void createShadowPipeline()
{
    // Load compiled shaders
    VkShaderModule vertModule = createShaderModule("tiny_engine_assets/shaders/shadow.vert.spv");
    VkShaderModule fragModule = createShaderModule("tiny_engine_assets/shaders/shadow.frag.spv");
    
    // Configure pipeline with depth-only rendering
    // ... (similar to createGraphicsPipeline but with shadow-specific settings)
    
    m_csm.shadowPipeline = vkCreateGraphicsPipelines(...);
}
```

## Data Structures

### ShadowUniformBufferObject
```cpp
struct ShadowUniformBufferObject {
    alignas(16) glm::mat4 lightSpaceMatrices[CASCADE_COUNT];
    alignas(16) glm::vec4 cascadeSplits;
    alignas(4) float shadowBias;
    alignas(4) float normalBias;
};
```

### CascadeData
```cpp
struct CascadeData {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 lightSpaceMatrix;
    glm::vec3 frustumNearCorner[8];
    glm::vec3 frustumFarCorner[8];
    float nearPlane;
    float farPlane;
};
```

## Shader Bindings

The modified fragment shader uses these bindings:
- Binding 0: UniformBufferObject (main scene data)
- Binding 1: Albedo texture sampler
- Binding 2: Metalness/Roughness texture sampler
- Binding 3: Normal map sampler
- **Binding 4: Shadow map array (sampler2DShadow)**
- **Binding 5: ShadowUniformBufferObject**

## Performance Considerations

1. **Shadow Map Resolution**: Higher resolution = better quality but more memory
   - 1024x1024: Fast, suitable for mobile/low-end
   - 2048x2048: Good balance (default)
   - 4096x4096: High quality, expensive

2. **Cascade Count**: More cascades = better quality distribution
   - 3 cascades: Minimum acceptable
   - 4 cascades: Recommended (default)
   - 5+ cascades: Diminishing returns

3. **PCF Kernel Size**: Larger kernels = softer shadows but slower
   - 3x3: Good performance (default)
   - 5x5: Better quality
   - Consider using VSM or ESM for very soft shadows

## Troubleshooting

### Shadow Acne
- Increase `SHADOW_BIAS` value
- Ensure proper polygon offset in rasterizer state

### Peter Panning
- Decrease `SHADOW_BIAS` if shadows detach from objects
- Check normal bias settings

### Cascade Boundary Artifacts
- Adjust `CASCADE_SPLIT_LAMBDA` for different split distribution
- Implement smooth cascade blending (advanced)

### Performance Issues
- Reduce shadow map resolution
- Decrease cascade count
- Reduce PCF kernel size
- Use viewport scissor testing for each cascade

## Advanced Features (Not Implemented)

For production use, consider adding:
1. **Cascade Blending** - Smooth transitions between cascades
2. **Stabilization** - Prevent shadow shimmering
3. **Variance Shadow Maps (VSM)** - Better filtering options
4. **Contact Hardening Shadows** - Distance-based penumbra size
5. **Cascaded Shadow Map Atlas** - Pack all cascades into single texture

## References

1. NVIDIA Developer: "Cascaded Shadow Maps" 
2. OpenGL Tutorial: "Shadow Mapping"
3. Vulkan Tutorial: "Depth buffering"
4. GPU Gems 3: "Practical Shadow Mapping"
