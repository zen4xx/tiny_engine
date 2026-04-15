#version 450
layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferCSM {
    mat4 lightViewProj[4];
    vec4 cascadeSplits; // x = split0, y = split1, z = split2, w = split3
    vec3 lightDir;
    int padding;   
} ubo_csm;

void main() {
    // В текущей реализации shadow pass вызывается 4 раза (для каждого каскада отдельно)
    // Поэтому используем lightViewProj[0], так как для каждого прохода обновляем UBO
    // с нужной матрицей, либо можно передавать индекс через push constant
    gl_Position = ubo_csm.lightViewProj[0] * pc.model * vec4(inPosition, 1.0);
}