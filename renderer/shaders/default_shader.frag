#version 450
#define MAX_POINT_LIGHTS_COUNT 16

layout(location = 0) in vec2  fragTexCoord;
layout(location = 1) in vec3  fragNormal;
layout(location = 2) in vec3  fragCameraPos;
layout(location = 3) in vec3  fragPos;
layout(location = 4) in vec3  fragTangent;
layout(location = 5) in vec3  fragBitangent;

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

// ----------------------------------------------------------------------------
// PBR helper functions
// ----------------------------------------------------------------------------

vec3 getNormalTangentSpace() {
    vec3 tex = texture(normalSampler, fragTexCoord).xyz;
    if(tex != vec3(0))
    {
        vec3 n = tex * 2.0 - 1.0;
        return normalize(mat3(normalize(fragTangent), normalize(fragBitangent), normalize(fragNormal)) * n);
    }
    else
        return normalize(fragNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = PI * (NdotH2 * (a2 - 1.0) + 1.0) * (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo    = texture(texSampler,     fragTexCoord).rgb;
    vec2 mr        = texture(mrSampler, fragTexCoord).bg;
    float metalness = mr.r;
    float roughness = mr.g;

    vec3 N = getNormalTangentSpace();
    vec3 V = normalize(fragCameraPos - fragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);

    vec3 Ld = normalize(-ubo.dirLight);
    vec3 Hd = normalize(V + Ld);
    float NDFd = DistributionGGX(N, Hd, roughness);
    float Gd   = GeometrySmith(N, V, Ld, roughness);
    vec3  Fd   = fresnelSchlick(max(dot(Hd, V), 0.0), F0);
    vec3  specular_d = NDFd * Gd * Fd
                      / (4.0 * max(dot(N, V), 0.0) * max(dot(N, Ld), 0.0) + 0.0001);
    vec3 kD_d = (vec3(1.0) - Fd) * (1.0 - metalness) + vec3(0.05);
    float NdotLd = max(dot(N, Ld), 0.0);
    vec3 irradiance_d = ubo.ambient + ubo.dirLightColor * NdotLd;

    vec3 Lo = (kD_d * albedo + specular_d) * irradiance_d;

    for(int i = 0; i < ubo.point_light_count; ++i) {
        vec3 lightPos   = ubo.point_light_pos[i].xyz;
        vec3 lightColor = ubo.point_light_colors[i].xyz;

        vec3 Lp = normalize(lightPos - fragPos);
        float dist = length(lightPos - fragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = lightColor * attenuation;


        vec3 Hp = normalize(V + Lp);
        float NDFp = DistributionGGX(N, Hp, roughness);
        float Gp   = GeometrySmith(N, V, Lp, roughness);
        vec3  Fp   = fresnelSchlick(max(dot(Hp, V), 0.0), F0);
        vec3  specular_p = NDFp * Gp * Fp
                          / (4.0 * max(dot(N, V), 0.0) * max(dot(N, Lp), 0.0) + 0.0001);
        vec3 kD_p = (vec3(1.0) - Fp) * (1.0 - metalness);
        float NdotLp = max(dot(N, Lp), 0.0);

        Lo += (kD_p * albedo + specular_p) * radiance * NdotLp;
    }

    outColor = vec4(Lo, 1.0);
}
