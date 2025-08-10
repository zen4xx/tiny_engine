#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;

    vec3 dirLight;
    vec3 dirLightColor;
    vec3 ambient;
} ubo;

layout(push_constant, std430) uniform pc {
    mat4 model;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;

layout(location = 2) out vec3 fragDirLight;
layout(location = 3) out vec3 fragAmbient;
layout(location = 4) out vec3 fragDirLightColor;

mat4 translate(mat4 m, vec3 v) {
    mat4 translation = mat4(1.0);
    translation[3][0] = v.x;
    translation[3][1] = v.y;
    translation[3][2] = v.z;
    return m * translation;
}

void main() {
    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(model))) * inNormal;
    fragAmbient = ubo.ambient;
    fragDirLight = normalize(ubo.dirLight);
    fragDirLightColor = ubo.dirLightColor;
}