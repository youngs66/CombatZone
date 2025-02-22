#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicRoughnessMap;
uniform sampler2D emissiveMap;
uniform sampler2D occlusionMap;
uniform sampler2D edgeMap;

// material properties
uniform vec4 baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec3 emissiveFactor;

// environment
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos;

const float PI = 3.14159265359;

// PBR functions
vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 detectEdges(sampler2D tex, vec2 uv) {
    float offset = 1.0 / 512.0;  // Adjust based on texture size
    
    // Sobel kernels
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2(0.0,  offset), vec2(offset,  offset),
        vec2(-offset,     0.0), vec2(0.0,     0.0), vec2(offset,     0.0),
        vec2(-offset, -offset), vec2(0.0, -offset), vec2(offset, -offset)
    );
    
    float kernelX[9] = float[](
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );
    
    float kernelY[9] = float[](
        -1, -2, -1,
         0,  0,  0,
         1,  2,  1
    );
    
    float edgeX = 0.0;
    float edgeY = 0.0;
    
    for(int i = 0; i < 9; i++) {
        float sample = texture(tex, uv + offsets[i]).r;
        edgeX += sample * kernelX[i];
        edgeY += sample * kernelY[i];
    }
    
    float edge = sqrt(edgeX * edgeX + edgeY * edgeY);
    return vec3(edge);
}

void main() {
    // Sample base color
    vec4 albedo = texture(albedoMap, TexCoords) * baseColorFactor;
    
    // Sample metallic-roughness
    vec2 metallicRoughness = texture(metallicRoughnessMap, TexCoords).bg;
    float metallic = metallicRoughness.x * metallicFactor;
    float roughness = metallicRoughness.y * roughnessFactor;
    
    // For now, just output the base color to debug
    FragColor = albedo;
} 