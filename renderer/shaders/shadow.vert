#version 450
// Shadow map vertex shader for CSM
// Renders depth from light's perspective

layout(binding = 0) uniform ShadowUniformBufferObject {
    mat4 lightSpaceMatrix;
} shadowUbo;

layout(push_constant, std430) uniform pc {
    mat4 model;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

void main() {
    gl_Position = shadowUbo.lightSpaceMatrix * model * vec4(inPosition, 1.0);
}
