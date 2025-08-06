#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 3) in vec3 fragDirLight;
layout(location = 4) in float fragAmbient;


layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

void main(){
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(fragDirLight - fragColor);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 result = fragAmbient + diffuse;

    outColor = texture(texSampler, fragTexCoord) * vec4(result, 1.0f);
}
