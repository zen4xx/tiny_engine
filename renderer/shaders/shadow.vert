#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 dirLight;
    vec3 dirLightColor;
    vec3 ambient;
    
    mat4 lightSpaceMatrix;
} ubo;

layout(push_constant, std430) uniform pc {
    mat4 model;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec4 fragPosLightSpace;

void main() {
    fragPosLightSpace = ubo.lightSpaceMatrix * model * vec4(inPosition, 1.0);
    gl_Position = fragPosLightSpace;
}
