#version 450

#define MAX_POINT_LIGHTS_COUNT 16

#define USE_NORMAL_MAP
// #define NORMALMAP_RECONSTRUCT_Z
// #define NORMALMAP_FLIP_GREEN
// #define USE_MR_RED_AS_AO
#define TONE_MAPPING_ACES
// #define GAMMA_CORRECTION
// #define ALBEDO_SRGB_DECODE
#define USE_ENVIRONMENT_REFLECTION

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragCameraPos;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;
layout(location = 6) in vec3 fragViewPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;
layout(set = 0, binding = 2) uniform sampler2D mrSampler;

#ifdef USE_NORMAL_MAP
layout(set = 0, binding = 3) uniform sampler2D normalSampler;
#endif

layout(set = 0, binding = 4) uniform sampler2DArrayShadow shadowSampler;
layout(set = 0, binding = 5) uniform sampler2D skyboxSampler;

const float PI = 3.14159265359;
const float MIN_ROUGHNESS = 0.05;

const float ENV_REFLECTION_INTENSITY = 0.2;
const float ENV_AMBIENT_INTENSITY = 0.05;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;

    vec3 dirLight;
    vec3 dirLightColor;
    vec3 ambient;

    vec4 point_light_colors[MAX_POINT_LIGHTS_COUNT];
    vec4 point_light_pos[MAX_POINT_LIGHTS_COUNT];

    int point_light_count;

    mat4 cascadeViewProjMatrices[4];
    vec4 cascadeSplits;
} ubo;

// --- PBR Lighting Functions ---

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
    float k = (r * r) / 8.0; // Direct lighting

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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 ShadeLight(
    vec3 L, vec3 V, vec3 N, vec3 radiance, vec3 albedo, vec3 F0, float metallic, float roughness
) {
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 diffuse = kD * albedo / PI;
    float denom = max(4.0 * NdotV * NdotL, 0.001);
    vec3 specular = (NDF * G * F) / denom;

    return (diffuse + specular) * radiance * NdotL;
}

float PCF3x3(vec3 shadowCoord, int cascadeIndex) {
    float shadow = 0.0;
    vec2 texelSize = vec2(1.0) / vec2(4096.0); 
    
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            vec3 sampleCoord = vec3(shadowCoord.xy + offset, cascadeIndex);
            shadow += texture(shadowSampler, vec4(sampleCoord, shadowCoord.z)).r;
        }
    }
    return shadow / 9.0;
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec2 VectorToLatLong(vec3 dir) {
    vec2 uv;
    uv.x = atan(dir.z, dir.x) * (0.5 / PI) + 0.5;
    uv.y = asin(dir.y) * (1.0 / PI) + 0.5;
    
    return uv;
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
    if (dot(N, N) < 1e-8) N = vec3(0.0, 0.0, 1.0);

#ifdef USE_NORMAL_MAP
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);

    T = normalize(T - dot(T, N) * N);
    B = normalize(B - dot(B, N) * N - dot(B, T) * T);

    mat3 TBN = mat3(T, B, N);
    vec3 normalTS = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;

#ifdef NORMALMAP_RECONSTRUCT_Z
    normalTS.z = sqrt(max(0.0, 1.0 - dot(normalTS.xy, normalTS.xy)));
#endif
#ifdef NORMALMAP_FLIP_GREEN
    normalTS.y = -normalTS.y;
#endif

    if (dot(normalTS, normalTS) < 1e-6) normalTS = vec3(0.0, 0.0, 1.0);
    N = normalize(TBN * normalize(normalTS));
#endif

    vec3 V = fragCameraPos - fragPos;
    float vLen = length(V);
    if (vLen < 1e-8) V = vec3(0.0, 0.0, 1.0); 
    else V /= vLen;

    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    // --- Directional Light + CSM Shadows ---
    {
        vec3 dir = ubo.dirLight;
        vec3 radiance = ubo.dirLightColor;

        if (dot(dir, dir) > 1e-8 && dot(radiance, radiance) > 0.0) {
            vec3 L = normalize(-dir);

            float shadow = 1.0;
            float viewDepth = abs(fragViewPos.z);
            
            int cascadeIndex = 3;
            if (viewDepth < ubo.cascadeSplits.x) cascadeIndex = 0;
            else if (viewDepth < ubo.cascadeSplits.y) cascadeIndex = 1;
            else if (viewDepth < ubo.cascadeSplits.z) cascadeIndex = 2;

            // Normal Offset Bias
            float normalOffsetScale = 0.15;
            vec3 biasedFragPos = fragPos + N * normalOffsetScale;

            vec4 shadowCoord = ubo.cascadeViewProjMatrices[cascadeIndex] * vec4(biasedFragPos, 1.0);
            shadowCoord.xyz = shadowCoord.xyz / shadowCoord.w;
            shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
            
            shadowCoord.z -= 0.0005; 

            shadow = PCF3x3(shadowCoord.xyz, cascadeIndex);
            Lo += shadow * ShadeLight(L, V, N, radiance, albedo, F0, metallic, roughness);
        }
    }

    // --- Point Lights ---
    int lightCount = clamp(ubo.point_light_count, 0, MAX_POINT_LIGHTS_COUNT);

    for (int i = 0; i < lightCount; ++i) {
        vec4 posRange = ubo.point_light_pos[i];
        vec3 toLight = posRange.xyz - fragPos;
        float dist = length(toLight);

        if (dist < 1e-6) continue;

        vec3 L = toLight / dist;
        float range = posRange.w;
        float attenuation = 0.0;

        if (range > 0.0) {
            if (dist < range) {
                // Smooth Windowing (Unreal-style attenuation)
                float window = 1.0 - pow(dist / range, 4.0);
                attenuation = (window * window) / max(dist * dist, 1e-6);
            }
        } else {
            attenuation = 1.0 / (1.0 + dist * dist);
        }

        if (attenuation <= 0.0) continue;

        vec3 radiance = ubo.point_light_colors[i].rgb * attenuation;
        if (dot(radiance, radiance) <= 0.0) continue;

        Lo += ShadeLight(L, V, N, radiance, albedo, F0, metallic, roughness);
    }

        // --- Ambient & Environment Approximation ---
    float ao = 1.0;
#ifdef USE_MR_RED_AS_AO
    ao = mr.r;
#endif

#ifdef USE_ENVIRONMENT_REFLECTION
    ivec2 skyboxSize = textureSize(skyboxSampler, 0);
    vec2 texelSize = 1.0 / vec2(skyboxSize);

    vec2 normUV = VectorToLatLong(N);
    vec3 diffuseEnvRaw = texture(skyboxSampler, normUV).rgb;
    
    vec3 diffuseEnv = diffuseEnvRaw * ENV_AMBIENT_INTENSITY;

    vec3 R = reflect(-V, N);
    vec2 reflUV = VectorToLatLong(R);
    vec3 reflectionColorRaw = vec3(0.0);

    float blurRadius = roughness * 15.0; 

    if (roughness < 0.05) {
        reflectionColorRaw = texture(skyboxSampler, reflUV).rgb;
    } else {
        float samples = 0.0;
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                vec2 offset = vec2(x, y) * texelSize * blurRadius;
                vec2 sampleUV = reflUV + offset;
                
                sampleUV.x = fract(sampleUV.x);
                sampleUV.y = clamp(sampleUV.y, 0.0, 1.0); 
                
                reflectionColorRaw += texture(skyboxSampler, sampleUV).rgb;
                samples += 1.0;
            }
        }
        reflectionColorRaw /= samples;
    }
    
    vec3 reflectionColor = reflectionColorRaw * ENV_REFLECTION_INTENSITY;

    vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 ambient = (kD * diffuseEnv + reflectionColor * kS) * ao;
    
    ambient += ubo.ambient * albedo * (1.0 - metallic) * ao;
#else
    vec3 ambient = ubo.ambient * albedo * (1.0 - metallic) * ao;
#endif

    vec3 color = ambient + Lo;

    // --- Post-Processing ---
#ifdef TONE_MAPPING_ACES
    color = ACESFilm(color);
#elif defined(TONE_MAPPING)
    color = color / (color + vec3(1.0));
#endif

#ifdef GAMMA_CORRECTION
    color = pow(color, vec3(1.0 / 2.2));
#endif

    outColor = vec4(color, baseColor.a);
}