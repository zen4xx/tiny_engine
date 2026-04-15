#version 450
// Shadow map fragment shader for CSM
// Outputs depth value to shadow map texture

void main() {
    // Store the depth value (gl_FragCoord.z) in the shadow map
    // The actual depth comparison is done by the GPU during rendering
}
