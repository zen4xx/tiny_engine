#version 450

#define MAX_POINT_LIGHTS_COUNT 16

// --- Uniform Buffer Object ---
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;

    vec3 dirLight;
    vec3 dirLightColor;
    vec3 ambient;

    vec4 point_light_colors[MAX_POINT_LIGHTS_COUNT];
    vec4 point_light_pos[MAX_POINT_LIGHTS_COUNT];

    int point_light_count;

    // CSM Data
    mat4 cascadeViewProjMatrices[4];
    vec4 cascadeSplits;
} ubo;

// --- Push Constants ---
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 lightSpaceMatrix;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragCameraPos;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;
layout(location = 6) out vec3 fragViewPos;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    gl_Position = ubo.proj * ubo.view * worldPos;

    fragCameraPos = -transpose(mat3(ubo.view)) * ubo.view[3].xyz;

    vec4 viewPos = ubo.view * worldPos;
    fragViewPos = viewPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    fragNormal = normalize(normalMatrix * inNormal);
    
    vec3 T = normalize(normalMatrix * inTangent.xyz);
    vec3 B = cross(fragNormal, T) * inTangent.w;
    
    fragTangent = T;
    fragBitangent = B;

    fragTexCoord = inTexCoord;
}