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
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec2  fragTexCoord;
layout(location = 1) out vec3  fragNormal;
layout(location = 2) out vec3  fragCameraPos;
layout(location = 3) out vec3  fragPos;
layout(location = 4) out vec3  fragTangent;
layout(location = 5) out vec3  fragBitangent;

void main() {
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 N = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent.xyz);
    vec3 B = normalize(cross(N, T) * inTangent.w);
    
    fragNormal    = N;
    fragTangent   = T;
    fragBitangent = B;

    fragTexCoord      = inTexCoord;
    fragPos           = vec3(model * vec4(inPosition, 1.0));
    fragCameraPos     = vec3(inverse(ubo.view) * vec4(0.0, 0.0, 0.0, 1.0));

    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
}