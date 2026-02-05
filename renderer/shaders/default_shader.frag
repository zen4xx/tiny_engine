#version 450
#define MAX_POINT_LIGHTS_COUNT 16
#define CASCADE_COUNT 4

// --- Входные данные ---
layout(location = 0) in vec2  fragTexCoord;
layout(location = 1) in vec3  fragNormal;
layout(location = 2) in vec3  fragCameraPos;
layout(location = 3) in vec3  fragPos;
layout(location = 4) in vec3  fragTangent;
layout(location = 5) in vec3  fragBitangent;

// --- Выход ---
layout(location = 0) out vec4 outColor;

// --- Текстуры ---
layout(binding = 1) uniform sampler2D texSampler;        // albedo
layout(binding = 2) uniform sampler2D mrSampler;         // metal/roughness
layout(binding = 3) uniform sampler2D normalSampler;     // normal map

// --- Основной UBO ---
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

// --- CSM UBO ---
layout(binding = 4) uniform CSMData {
    mat4 lightViewProj[CASCADE_COUNT];
    vec4 cascadeSplits; // x = split0, y = split1, z = split2, w = split3
    vec3 lightDir;
    int cascadeCount;
} csm;

// --- Теневые карты (sampler2DShadow) ---
layout(binding = 5) uniform sampler2DShadow shadowMap0;
layout(binding = 6) uniform sampler2DShadow shadowMap1;
layout(binding = 7) uniform sampler2DShadow shadowMap2;
layout(binding = 8) uniform sampler2DShadow shadowMap3;

// --- Настройки теней ---
const float SHADOW_BIAS = 0.005f;
const int PCF_KERNEL_SIZE = 2;
const float PCF_STEP = 1.0 / 2048.0; // соответствует разрешению shadow map из патча
const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// PBR helper functions (без изменений)
// ----------------------------------------------------------------------------
vec3 getNormalTangentSpace() {
    vec3 tex = texture(normalSampler, fragTexCoord).xyz;
    if (tex != vec3(0)) {
        vec3 n = tex * 2.0 - 1.0;
        return normalize(mat3(normalize(fragTangent), normalize(fragBitangent), normalize(fragNormal)) * n);
    } else {
        return normalize(fragNormal);
    }
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float denom = PI * (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom *= denom;
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
// Теневая функция с PCF
// ----------------------------------------------------------------------------
float sampleShadowPCF(sampler2DShadow shadowMap, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // [-1,1] → [0,1]

    if (projCoords.z > 1.0 || any(lessThan(projCoords.xy, vec2(0.0))) || any(greaterThan(projCoords.xy, vec2(1.0))))
        return 1.0; // вне тени

    float shadow = 0.0;
    for (int x = -PCF_KERNEL_SIZE; x <= PCF_KERNEL_SIZE; ++x)
    {
        for (int y = -PCF_KERNEL_SIZE; y <= PCF_KERNEL_SIZE; ++y)
        {
            vec2 offset = vec2(x, y) * PCF_STEP;
            shadow += texture(shadowMap, vec3(projCoords.xy + offset, projCoords.z - SHADOW_BIAS));
        }
    }
    return shadow / (pow(2 * PCF_KERNEL_SIZE + 1, 2));
}

// ----------------------------------------------------------------------------
// Основная функция
// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo    = texture(texSampler, fragTexCoord).rgb;
    vec2 mr        = texture(mrSampler, fragTexCoord).bg;
    float metalness = mr.r;
    float roughness = mr.g;
    vec3 N = getNormalTangentSpace();
    vec3 V = normalize(fragCameraPos - fragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);

    float distFromCam = length(fragPos - fragCameraPos);
    int cascadeIndex = 0;
    if (distFromCam > csm.cascadeSplits.x) cascadeIndex = 1;
    if (distFromCam > csm.cascadeSplits.y) cascadeIndex = 2;
    if (distFromCam > csm.cascadeSplits.z) cascadeIndex = 3;

    vec4 fragPosLightSpace;
    float shadowFactor;

    if (cascadeIndex == 0) {
        fragPosLightSpace = csm.lightViewProj[0] * vec4(fragPos, 1.0);
        shadowFactor = sampleShadowPCF(shadowMap0, fragPosLightSpace);
    } else if (cascadeIndex == 1) {
        fragPosLightSpace = csm.lightViewProj[1] * vec4(fragPos, 1.0);
        shadowFactor = sampleShadowPCF(shadowMap1, fragPosLightSpace);
    } else if (cascadeIndex == 2) {
        fragPosLightSpace = csm.lightViewProj[2] * vec4(fragPos, 1.0);
        shadowFactor = sampleShadowPCF(shadowMap2, fragPosLightSpace);
    } else { // cascadeIndex == 3
        fragPosLightSpace = csm.lightViewProj[3] * vec4(fragPos, 1.0);
        shadowFactor = sampleShadowPCF(shadowMap3, fragPosLightSpace);
    }

    // === Освещение ===
    vec3 Lo = ubo.ambient * albedo; // ambient

    // Направленный свет
    vec3 Ld = normalize(-ubo.dirLight); // направление к источнику
    vec3 Hd = normalize(V + Ld);
    float NDFd = DistributionGGX(N, Hd, roughness);
    float Gd   = GeometrySmith(N, V, Ld, roughness);
    vec3 Fd    = fresnelSchlick(max(dot(Hd, V), 0.0), F0);
    vec3 kS_d  = Fd;
    vec3 kD_d  = (vec3(1.0) - Fd) * (1.0 - metalness);
    float NdotLd = max(dot(N, Ld), 0.0);
    vec3 radianceDir = ubo.dirLightColor * NdotLd;
    Lo += (kD_d * albedo / PI + kS_d * (NDFd * Gd / (4.0 * max(dot(N, V), 0.001) * max(dot(N, Ld), 0.001)))) * radianceDir * shadowFactor;

    // Точечные источники
    for (int i = 0; i < ubo.point_light_count; ++i)
    {
        vec3 lightPos   = ubo.point_light_pos[i].xyz;
        vec3 lightColor = ubo.point_light_colors[i].xyz;
        vec3 Lp = normalize(lightPos - fragPos);
        float dist = length(lightPos - fragPos);
        float attenuation = 1.0 / (dist * dist + 0.01); // небольшой offset для стабильности
        vec3 radiance = lightColor * attenuation;
        vec3 Hp = normalize(V + Lp);
        float NDFp = DistributionGGX(N, Hp, roughness);
        float Gp   = GeometrySmith(N, V, Lp, roughness);
        vec3 Fp    = fresnelSchlick(max(dot(Hp, V), 0.0), F0);
        vec3 kS_p  = Fp;
        vec3 kD_p  = (vec3(1.0) - Fp) * (1.0 - metalness);
        float NdotLp = max(dot(N, Lp), 0.0);
        Lo += (kD_p * albedo / PI + kS_p * (NDFp * Gp / (4.0 * max(dot(N, V), 0.001) * max(dot(N, Lp), 0.001)))) * radiance * NdotLp;
    }

    outColor = vec4(Lo, 1.0);
}