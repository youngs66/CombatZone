#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D metallicRoughnessMap;
uniform vec4 baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

void main() {
    // Sample base color
    vec4 color = texture(albedoMap, TexCoords);
    
    // Apply material properties
    color *= baseColorFactor;
    
    // Ensure alpha is properly handled
    if (color.a < 0.1) {
        discard;
    }
    
    FragColor = color;
} 