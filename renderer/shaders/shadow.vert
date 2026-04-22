#version 450
layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;

layout(binding = 4) uniform UniformBufferCSM {
    mat4 lightViewProj[4];
} ubo_csm;

void main() {
    gl_Position = ubo_csm.lightViewProj[0] * pc.model * vec4(inPosition, 1.0);
}