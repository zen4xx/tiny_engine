#version 450

#define MAX_POINT_LIGHTS_COUNT 16
// #define USE_NORMAL_MAP
// #define NORMALMAP_RECONSTRUCT_Z
// #define NORMALMAP_FLIP_GREEN
// #define USE_MR_RED_AS_AO
// #define TONE_MAPPING
// #define GAMMA_CORRECTION
// #define ALBEDO_SRGB_DECODE

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragCameraPos;
layout(location = 3) in vec3 fragPos;

#ifdef USE_NORMAL_MAP
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;
#endif

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;
layout(set = 0, binding = 2) uniform sampler2D mrSampler;

#ifdef USE_NORMAL_MAP
layout(set = 0, binding = 3) uniform sampler2D normalSampler;
#endif

const float PI = 3.14159265359;
const float MIN_ROUGHNESS = 0.05;

layout(std140, set = 0, binding = 0) uniform UniformBufferObject {
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
    float NdotL = max(dot(N, L), 0.0);

    if (NdotL <= 0.0) {
        return vec3(0.0);
    }

    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 diffuse = kD * albedo / PI;

    float denom = max(4.0 * NdotV * NdotL, 1e-4);
    vec3 specular = (NDF * G * F) / denom;

    return (diffuse + specular) * radiance * NdotL;
}

void main() {
    vec4 baseColor = texture(texSampler, fragTexCoord);

#ifdef ALBEDO_SRGB_DECODE
    baseColor.rgb = pow(baseColor.rgb, vec3(2.2));
#endif

    vec4 mr = texture(mrSampler, fragTexCoord);

    vec3 albedo = baseColor.rgb;

    float metallic = clamp(mr.b, 0.0, 1.0);
    float roughness = clamp(mr.g, MIN_ROUGHNESS, 1.0);

    vec3 N = normalize(fragNormal);

    if (dot(N, N) < 1e-8) {
        N = vec3(0.0, 0.0, 1.0);
    }

#ifdef USE_NORMAL_MAP
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);

    if (dot(T, T) < 1e-8) {
        vec3 up = (abs(N.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, N));
    } else {
        T = normalize(T - dot(T, N) * N);
    }

    vec3 C = cross(N, T);

    if (dot(C, C) < 1e-8) {
        vec3 up = (abs(N.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, N));
        C = cross(N, T);
    }

    float handedness = 1.0;

    if (dot(B, B) > 1e-8) {
        handedness = (dot(C, B) < 0.0) ? -1.0 : 1.0;
    }

    B = normalize(C * handedness);

    mat3 TBN = mat3(T, B, N);

    vec3 normalTS = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;

#ifdef NORMALMAP_RECONSTRUCT_Z
    normalTS.z = sqrt(max(0.0, 1.0 - dot(normalTS.xy, normalTS.xy)));
#endif

#ifdef NORMALMAP_FLIP_GREEN
    normalTS.y = -normalTS.y;
#endif

    if (dot(normalTS, normalTS) < 1e-6) {
        normalTS = vec3(0.0, 0.0, 1.0);
    }

    N = normalize(TBN * normalize(normalTS));
#endif

    vec3 V = fragCameraPos - fragPos;
    float vLen = length(V);

    if (vLen < 1e-8) {
        V = vec3(0.0, 0.0, 1.0);
    } else {
        V /= vLen;
    }

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Directional light
    {
        vec3 dir = ubo.dirLight;
        vec3 radiance = ubo.dirLightColor;

        if (dot(dir, dir) > 1e-8 && dot(radiance, radiance) > 0.0) {
            vec3 L = normalize(-dir);

            Lo += ShadeLight(
                L,
                V,
                N,
                radiance,
                albedo,
                F0,
                metallic,
                roughness
            );
        }
    }

    // Point lights
    int lightCount = ubo.point_light_count;

    if (lightCount < 0) {
        lightCount = 0;
    }

    if (lightCount > MAX_POINT_LIGHTS_COUNT) {
        lightCount = MAX_POINT_LIGHTS_COUNT;
    }

    for (int i = 0; i < lightCount; ++i) {
        vec4 posRange = ubo.point_light_pos[i];

        vec3 toLight = posRange.xyz - fragPos;
        float dist = length(toLight);

        if (dist < 1e-6) {
            continue;
        }

        vec3 L = toLight / dist;

        float range = posRange.w;
        float attenuation = 0.0;

        if (range > 0.0) {
            if (dist < range) {
                float window = 1.0 - pow(dist / range, 4.0);
                attenuation = (window * window) / max(dist * dist, 1e-6);
            } else {
                attenuation = 0.0;
            }
        } else {
            attenuation = 1.0 / (1.0 + dist * dist);
        }

        if (attenuation <= 0.0) {
            continue;
        }

        vec3 radiance = ubo.point_light_colors[i].rgb * attenuation;

        if (dot(radiance, radiance) <= 0.0) {
            continue;
        }

        Lo += ShadeLight(
            L,
            V,
            N,
            radiance,
            albedo,
            F0,
            metallic,
            roughness
        );
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

#ifdef GAMMA_CORRECTION
    color = pow(color, vec3(1.0 / 2.2));
#endif

    outColor = vec4(color, baseColor.a);
}