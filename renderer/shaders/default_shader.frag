#version 450

layout(location = 0) in vec2  fragTexCoord;
layout(location = 1) in vec3  fragNormal;
layout(location = 2) in vec3  fragDirLight;
layout(location = 3) in vec3  fragAmbient;
layout(location = 4) in vec3  fragDirLightColor;
layout(location = 5) in vec3  fragCameraPos;
layout(location = 6) in vec3  fragPos;
layout(location = 7) in vec3  fragTangent;
layout(location = 8) in vec3  fragBitangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;    
layout(binding = 2) uniform sampler2D mrSampler;      
layout(binding = 3) uniform sampler2D normalSampler;  

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// PBR helper functions
// ----------------------------------------------------------------------------

vec3 getNormalTangentSpace() {
    vec3 n = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;
    return normalize(mat3(normalize(fragTangent), normalize(fragBitangent), normalize(fragNormal)) * n);
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
    vec3 albedo = pow(texture(texSampler, fragTexCoord).rgb, vec3(2.2)); 
    vec2 mr = texture(mrSampler, fragTexCoord).bg;
    float metalness = mr.r;
    float roughness = mr.g;

    vec3 N = getNormalTangentSpace();
    vec3 V = normalize(fragCameraPos - fragPos);
    vec3 L = normalize(-fragDirLight);
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = nominator / denominator;

    vec3 kD = (vec3(1.0) - F) * (1.0 - metalness);
    float NdotL = max(dot(N, L), 0.0);
    vec3 irradiance = fragAmbient + fragDirLightColor * NdotL;

    kD += 0.05; 

    vec3 Lo = (kD * albedo + specular) * irradiance;
    
    outColor = vec4(Lo, 1.0);
}
