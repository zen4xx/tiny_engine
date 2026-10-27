#version 450

#define MAX_POINT_LIGHTS_COUNT 16

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragCameraPos;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D mrSampler;
layout(binding = 3) uniform sampler2D normalSampler;

const float PI = 3.14159265359;

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

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0) + 1.0;

    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    return GeometrySchlickGGX(NdotV, roughness) *
           GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 ShadeLight(
    vec3 L,
    vec3 V,
    vec3 N,
    vec3 radiance,
    vec3 albedo,
    vec3 F0,
    float metallic,
    float roughness
) {
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    vec3 diffuse = kD * albedo / PI;

    vec3 specular = (NDF * G * F) / max(4.0 * NdotV * NdotL, 0.001);

    return (diffuse + specular) * radiance * NdotL;
}

void main() {
    vec4 baseColor = texture(texSampler, fragTexCoord);
    vec4 mr = texture(mrSampler, fragTexCoord);

    vec3 albedo = baseColor.rgb;

    float metallic = clamp(mr.b, 0.0, 1.0);
    float roughness = clamp(mr.g, 0.05, 1.0);

    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);

    T = normalize(T - dot(T, N) * N);
    float tbnHandedness = (dot(cross(N, T), B) < 0.0) ? -1.0 : 1.0;
    B = normalize(cross(N, T) * tbnHandedness);

#ifdef USE_NORMAL_MAP
    mat3 TBN = mat3(T, B, N);

    vec3 normalTS = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;

    if (dot(normalTS, normalTS) < 1e-6) {
        normalTS = vec3(0.0, 0.0, 1.0);
    }

    N = normalize(TBN * normalize(normalTS));
#else
    N = normalize(fragNormal);
#endif

    vec3 V = normalize(fragCameraPos - fragPos);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    {
        vec3 dir = ubo.dirLight;

        vec3 L = (dot(dir, dir) > 1e-8) ? normalize(-dir) : vec3(0.0, 1.0, 0.0);

        vec3 radiance = ubo.dirLightColor;

        Lo += ShadeLight(L, V, N, radiance, albedo, F0, metallic, roughness);
    }

    for (int i = 0; i < MAX_POINT_LIGHTS_COUNT; ++i) {
        if (i >= ubo.point_light_count) {
            break;
        }

        vec3 lightPos = ubo.point_light_pos[i].xyz;
        vec3 toLight = lightPos - fragPos;

        float dist = max(length(toLight), 1e-3);
        vec3 L = normalize(toLight);

        float range = ubo.point_light_pos[i].w;

        float attenuation;

        if (range > 0.0) {
            float window = clamp(1.0 - pow(dist / range, 4.0), 0.0, 1.0);
            attenuation = (window * window) / (dist * dist);
        } else {
            attenuation = 1.0 / (1.0 + dist * dist);
        }

        float intensity = ubo.point_light_colors[i].a;

        if (intensity <= 0.0) {
            intensity = 1.0;
        }

        vec3 radiance = ubo.point_light_colors[i].rgb * intensity * attenuation;

        Lo += ShadeLight(L, V, N, radiance, albedo, F0, metallic, roughness);
    }

    float ao = 1.0;

#ifdef USE_MR_RED_AS_AO
    ao = mr.r;
#endif

    vec3 ambient = ubo.ambient * albedo * (1.0 - metallic) * ao;

    vec3 color = ambient + Lo;

#ifdef TONE_MAPPING
    color = color / (color + vec3(1.0));
#endif

    outColor = vec4(color, baseColor.a);
}