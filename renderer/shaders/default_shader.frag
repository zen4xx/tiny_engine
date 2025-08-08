#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(location = 2) in vec3 fragDirLight;
layout(location = 3) in vec3 fragAmbient;
layout(location = 4) in vec3 fragDirLightColor;


layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

void main(){
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, fragDirLight), 0.0);
    vec3 diffuse = diff * fragDirLightColor;

    vec3 result = fragAmbient + diffuse;

    outColor = texture(texSampler, fragTexCoord) * vec4(result, 1.0f);
}
