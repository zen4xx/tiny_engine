#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(push_constant) uniform PushConstants {
    mat4 cascadeViewProj;
} pc;
void main() {
    gl_Position = pc.cascadeViewProj * vec4(inPos, 1.0);
}