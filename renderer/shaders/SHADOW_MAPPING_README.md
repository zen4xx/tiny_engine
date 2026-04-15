# Shadow Mapping Implementation

## Overview
This implementation adds shadow mapping to the Vulkan-based renderer using directional light shadows.

## Files Created/Modified

### New Shader Files:
1. **shadow.vert** - Vertex shader for shadow map rendering pass
   - Transforms vertices to light space using `lightSpaceMatrix`
   - Outputs position in light space for depth comparison

2. **shadow.frag** - Fragment shader for shadow map rendering pass
   - Empty fragment shader (depth-only rendering)

### Modified Shader Files:
1. **default_shader.vert** - Main rendering vertex shader
   - Added `sampler2DShadow` binding (binding = 4)
   - Added `fragPosLightSpace` output (location = 6)

2. **default_shader.frag** - Main rendering fragment shader
   - Added `lightSpaceMatrix` to UBO
   - Added `fragPosLightSpace` input
   - Added `sampler2DShadow` for shadow map sampling
   - Implemented `calculateShadow()` function with PCF (Percentage Closer Filtering)
   - Applied shadow factor to directional light contribution

## Key Components

### Shadow Map Calculation
```glsl
float calculateShadow(vec3 fragPosLightSpace) {
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Depth comparison with bias
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    
    // PCF soft shadows (3x3 kernel)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(2048.0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}
```

### Uniform Buffer Changes
The UBO now includes:
```glsl
mat4 lightSpaceMatrix;  // View-Projection matrix from light's perspective
```

### Descriptor Set Layout
Binding 4 is now used for the shadow map sampler:
```glsl
layout(binding = 4) uniform sampler2DShadow shadowMap;
```

## Integration Steps Required

To complete the integration, you need to:

1. **Create Shadow Map Resources** (in renderer.cpp):
   - Create depth image for shadow map (2048x2048 or higher)
   - Create image view for shadow map
   - Create descriptor set for shadow map sampler

2. **Create Shadow Render Pass**:
   - Create render pass for depth-only rendering
   - Create graphics pipeline for shadow pass
   - Configure depth compare operation

3. **Calculate Light Space Matrix**:
   ```cpp
   glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
   glm::mat4 lightView = glm::lookAt(lightPos, target, up);
   glm::mat4 lightSpaceMatrix = lightProjection * lightView;
   ```

4. **Render Shadow Pass** (before main render pass):
   - Bind shadow pipeline
   - Update lightSpaceMatrix in UBO
   - Render all objects that cast shadows

5. **Update Main Render Pass**:
   - Bind shadow map to descriptor set binding 4
   - Ensure UBO includes lightSpaceMatrix

## Parameters to Tune

- **Shadow Map Resolution**: Currently assumed 2048x2048 in PCF calculation
- **Bias Value**: Currently 0.005, adjust to reduce shadow acne or peter-panning
- **PCF Kernel Size**: Currently 3x3, can be increased for softer shadows
- **Light Ortho Bounds**: Adjust based on your scene size

## Common Issues & Solutions

1. **Shadow Acne**: Increase bias value or use slope-scaled bias
2. **Peter-Panning**: Decrease bias or move geometry slightly towards light
3. **Aliasing**: Increase shadow map resolution or use CSM (Cascaded Shadow Maps)
4. **Hard Edges**: Increase PCF kernel size or use VSM/ESM techniques
