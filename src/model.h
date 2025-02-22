#pragma once
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE

#include "../external/tinygltf/tiny_gltf.h"
#include "shader.h"
#include <vector>
#include <string>
#include <filesystem>
#include "../external/glm/glm/glm.hpp"
#include <map>

class Model {
public:
    Model(const char* path);
    void Draw(Shader &shader);

private:
    struct Texture {
        GLuint id;
        std::string type;
        std::string path;  // For debugging
    };

    struct Material {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        bool doubleSided = false;
        glm::vec3 emissiveFactor = glm::vec3(0.0f);
        
        std::vector<Texture> textures;
        std::map<std::string, GLuint> textureMap;  // Maps texture types to texture IDs
    };

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;
    Material material;  // Add material member
    
    void loadModel(const char* path);
    void setupMesh();
    GLuint loadTexture(const tinygltf::Image& image);
}; 