#version 450

#define MAX_POINT_LIGHTS_COUNT 16

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;

    vec3 dirLight;
    vec3 dirLightColor;
    vec3 ambient;

    vec4 point_light_colors[MAX_POINT_LIGHTS_COUNT];
    vec4 point_light_pos[MAX_POINT_LIGHTS_COUNT];

    int point_light_count;
} ubo;

layout(push_constant, std430) uniform pc {
    mat4 model;
};

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

void main() {
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    mat3 model3 = mat3(model);
    mat3 normalMatrix = transpose(inverse(model3));

    vec3 N = normalize(normalMatrix * inNormal);

    float mirror = (determinant(model3) < 0.0) ? -1.0 : 1.0;

    vec3 T;
    float handedness = inTangent.w * mirror;

    bool useTangent = (length(inTangent.xyz) > 1e-5) && (abs(handedness) > 1e-5);

    if (useTangent) {
        T = normalize(model3 * inTangent.xyz);

        T -= dot(T, N) * N;

        if (length(T) < 1e-5) {
            useTangent = false;
        } else {
            T = normalize(T);
        }
    }

    if (!useTangent) {
        vec3 helper = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(helper, N));
        handedness = mirror;
    }

    // glTF: bitangent = cross(normal, tangent) * tangent.w
    vec3 B = normalize(cross(N, T) * handedness);

    fragNormal = N;
    fragTangent = T;
    fragBitangent = B;
    fragTexCoord = inTexCoord;

    fragCameraPos = inverse(ubo.view)[3].xyz;

    gl_Position = ubo.proj * ubo.view * worldPos;
}