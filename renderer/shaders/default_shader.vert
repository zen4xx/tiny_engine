#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;

    vec3 dirLight;
    vec3 ambient;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragDirLight;
layout(location = 4) out vec3 fragAmbient;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = vec3(0.0f, 0.0f, 0.0f);
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragDirLight = ubo.dirLight;
    fragAmbient = ubo.ambient;
}