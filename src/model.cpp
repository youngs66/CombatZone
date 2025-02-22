#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"
#include "model.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>  // Add this for memcpy

Model::Model(const char* path) {
    loadModel(path);
    setupMesh();
}

void Model::loadModel(const char* path) {
    // Add file existence check
    std::ifstream file(path);
    if (!file.good()) {
        std::cout << "Error: Cannot find model file at: " << path << std::endl;
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        return;
    }
    file.close();

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Configure TinyGLTF
    loader.SetPreserveImageChannels(true);  // Keep original image format

    // Set up image loader callback
    loader.SetImageLoader([](tinygltf::Image* image, const int imageIndex,
                           std::string* error, std::string* warning, int req_width,
                           int req_height, const unsigned char* bytes, int size,
                           void* userData) -> bool {
        if (!bytes) {
            if (error) *error = "Image bytes are null";
            return false;
        }

        std::cout << "\nImage Loader Debug:" << std::endl;
        std::cout << "Data size: " << size << " bytes" << std::endl;
        std::cout << "First few bytes: ";
        for(int i = 0; i < std::min(16, size); i++) {
            std::cout << std::hex << (int)bytes[i] << " ";
        }
        std::cout << std::dec << std::endl;

        // For PNG files, we need to decode them
        if (size > 8 && bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4E && bytes[3] == 0x47) {
            std::cout << "Detected PNG format" << std::endl;
            
            int width, height, channels;
            unsigned char* decoded = stbi_load_from_memory(bytes, size, &width, &height, &channels, STBI_rgb_alpha);
            
            if (!decoded) {
                if (error) *error = "Failed to decode PNG image";
                return false;
            }

            std::cout << "Decoded PNG: " << width << "x" << height << ", " << channels << " channels" << std::endl;

            // Set image properties
            image->width = width;
            image->height = height;
            image->component = 4;  // RGBA
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

            // Copy decoded data
            size_t img_size = width * height * 4;
            image->image.resize(img_size);
            std::memcpy(image->image.data(), decoded, img_size);

            stbi_image_free(decoded);
            return true;
        }

        // For raw image data
        if (req_width > 0 && req_height > 0) {
            image->width = req_width;
            image->height = req_height;
            image->component = 4;
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

            size_t img_size = req_width * req_height * 4;
            image->image.resize(img_size);
            std::memcpy(image->image.data(), bytes, std::min(size, (int)img_size));
            return true;
        }

        if (error) *error = "Unsupported image format";
        return false;
    }, nullptr);

    bool ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!ret) {
        std::cout << "Failed to load GLB file: " << path << std::endl;
        return;
    }

    // Process all meshes in the model
    for (const auto& mesh : gltfModel.meshes) {
        std::cout << "Processing mesh with " << mesh.primitives.size() << " primitives" << std::endl;
        
        for (const auto& primitive : mesh.primitives) {
            size_t startIndex = vertices.size() / 8;  // Current vertex count
            
            // Get vertex positions
            const auto& posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
            const auto& posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = gltfModel.buffers[posBufferView.buffer];

            // Get vertex normals if they exist
            const auto& normalAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
            const auto& normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
            const auto& normalBuffer = gltfModel.buffers[normalBufferView.buffer];

            // Get texture coordinates if they exist
            const auto& texcoordAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto& texcoordBufferView = gltfModel.bufferViews[texcoordAccessor.bufferView];
            const auto& texcoordBuffer = gltfModel.buffers[texcoordBufferView.buffer];

            size_t vertexCount = posAccessor.count;
            size_t oldVertSize = vertices.size();
            vertices.resize(oldVertSize + vertexCount * 8);

            // Copy data for this primitive
            const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset]);
            const float* normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset]);
            const float* texcoordData = reinterpret_cast<const float*>(&texcoordBuffer.data[texcoordBufferView.byteOffset]);

            for (size_t i = 0; i < vertexCount; i++) {
                size_t vIdx = (oldVertSize / 8 + i) * 8;
                // Position
                vertices[vIdx + 0] = posData[i * 3 + 0];
                vertices[vIdx + 1] = posData[i * 3 + 1];
                vertices[vIdx + 2] = posData[i * 3 + 2];
                // Normal
                vertices[vIdx + 3] = normalData[i * 3 + 0];
                vertices[vIdx + 4] = normalData[i * 3 + 1];
                vertices[vIdx + 5] = normalData[i * 3 + 2];
                // Texcoord
                vertices[vIdx + 6] = texcoordData[i * 2 + 0];
                vertices[vIdx + 7] = texcoordData[i * 2 + 1];
            }

            // Load indices
            const auto& indicesAccessor = gltfModel.accessors[primitive.indices];
            const auto& indicesBufferView = gltfModel.bufferViews[indicesAccessor.bufferView];
            const auto& indicesBuffer = gltfModel.buffers[indicesBufferView.buffer];

            size_t indexCount = indicesAccessor.count;
            size_t oldIdxSize = indices.size();
            indices.resize(oldIdxSize + indexCount);

            const uint16_t* indexData = reinterpret_cast<const uint16_t*>(&indicesBuffer.data[indicesBufferView.byteOffset]);
            for (size_t i = 0; i < indexCount; i++) {
                indices[oldIdxSize + i] = indexData[i] + startIndex;  // Offset indices for this primitive
            }
        }
    }

    // Print debug info
    std::cout << "\nModel Statistics:" << std::endl;
    std::cout << "Total vertices: " << vertices.size() / 8 << std::endl;
    std::cout << "Total indices: " << indices.size() << std::endl;
    std::cout << "Number of meshes: " << gltfModel.meshes.size() << std::endl;

    // Add debug output after loading
    std::cout << "Model loaded successfully!" << std::endl;
    std::cout << "Number of textures: " << textures.size() << std::endl;

    // Add vertex data debug output
    std::cout << "\nFirst vertex data:" << std::endl;
    for(int i = 0; i < 8; i++) {
        std::cout << vertices[i] << " ";
    }
    std::cout << "\nFirst three indices:" << std::endl;
    for(int i = 0; i < 3; i++) {
        std::cout << indices[i] << " ";
    }
    std::cout << std::endl;

    // Load textures if they exist
    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.material >= 0) {
                const auto& material = gltfModel.materials[primitive.material];
                
                std::cout << "\nMaterial Debug:" << std::endl;
                std::cout << "Material index: " << primitive.material << std::endl;
                
                // Print all available textures in the material
                if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    const auto& texture = gltfModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
                    const auto& image = gltfModel.images[texture.source];
                    
                    std::cout << "\nTexture Loading Debug:" << std::endl;
                    std::cout << "Texture index: " << material.pbrMetallicRoughness.baseColorTexture.index << std::endl;
                    std::cout << "Image source: " << texture.source << std::endl;
                    std::cout << "Image dimensions: " << image.width << "x" << image.height << std::endl;
                    std::cout << "Image components: " << image.component << std::endl;
                    std::cout << "Image data size: " << image.image.size() << std::endl;
                    
                    if (image.width > 0 && image.height > 0 && !image.image.empty()) {
                        Texture tex;
                        tex.id = loadTexture(image);
                        tex.type = "texture_diffuse1";
                        tex.path = image.uri;
                        textures.push_back(tex);
                        std::cout << "Successfully loaded texture with ID: " << tex.id << std::endl;
                    } else {
                        std::cout << "Invalid image data, skipping texture" << std::endl;
                    }
                } else {
                    std::cout << "No base color texture" << std::endl;
                }
                
                if (material.occlusionTexture.index >= 0) {
                    const auto& texture = gltfModel.textures[material.occlusionTexture.index];
                    const auto& image = gltfModel.images[texture.source];
                    
                    Texture tex;
                    tex.id = loadTexture(image);
                    tex.type = "texture_ambient1";
                    textures.push_back(tex);
                    std::cout << "Loaded ambient occlusion texture" << std::endl;
                } else {
                    std::cout << "No occlusion texture" << std::endl;
                }
                
                // Print base color factor
                const auto& baseColor = material.pbrMetallicRoughness.baseColorFactor;
                std::cout << "Base color factor: "
                          << baseColor[0] << ", "
                          << baseColor[1] << ", "
                          << baseColor[2] << ", "
                          << baseColor[3] << std::endl;
                
                // Print all available images
                std::cout << "Number of images in model: " << gltfModel.images.size() << std::endl;
                for (size_t i = 0; i < gltfModel.images.size(); i++) {
                    const auto& image = gltfModel.images[i];
                    std::cout << "Image " << i << ": "
                              << "Size: " << image.width << "x" << image.height
                              << ", Components: " << image.component
                              << ", Data size: " << image.image.size() << std::endl;
                }
            }
        }
    }

    // Load the edge detection texture
    int edgeWidth, edgeHeight, edgeChannels;
    unsigned char* edgeData = stbi_load("assets/images/tank_hires_edge.png", 
                                      &edgeWidth, &edgeHeight, &edgeChannels, 0);
    if (edgeData) {
        GLuint edgeTextureID;
        glGenTextures(1, &edgeTextureID);
        glBindTexture(GL_TEXTURE_2D, edgeTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, edgeWidth, edgeHeight, 0, 
                     GL_RED, GL_UNSIGNED_BYTE, edgeData);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(edgeData);
        
        // Store the edge texture
        Texture edgeTex;
        edgeTex.id = edgeTextureID;
        edgeTex.type = "edgeMap";
        textures.push_back(edgeTex);
    }

    if (gltfModel.materials.size() > 0) {
        const auto& glTFMaterial = gltfModel.materials[0];  // Use first material
        
        std::cout << "\nMaterial Debug:" << std::endl;
        
        // Load double-sided property
        material.doubleSided = glTFMaterial.doubleSided;
        std::cout << "Double Sided: " << (material.doubleSided ? "true" : "false") << std::endl;
        
        // Load base color factor
        if (glTFMaterial.pbrMetallicRoughness.baseColorFactor.size() == 4) {
            material.baseColorFactor = glm::vec4(
                glTFMaterial.pbrMetallicRoughness.baseColorFactor[0],
                glTFMaterial.pbrMetallicRoughness.baseColorFactor[1],
                glTFMaterial.pbrMetallicRoughness.baseColorFactor[2],
                glTFMaterial.pbrMetallicRoughness.baseColorFactor[3]
            );
            std::cout << "Base Color Factor: "
                      << material.baseColorFactor.r << ", "
                      << material.baseColorFactor.g << ", "
                      << material.baseColorFactor.b << ", "
                      << material.baseColorFactor.a << std::endl;
        }
        
        // Load metallic factor
        material.metallicFactor = glTFMaterial.pbrMetallicRoughness.metallicFactor;
        std::cout << "Metallic Factor: " << material.metallicFactor << std::endl;
        
        // Load roughness factor
        material.roughnessFactor = glTFMaterial.pbrMetallicRoughness.roughnessFactor;
        std::cout << "Roughness Factor: " << material.roughnessFactor << std::endl;
        
        // Load textures with proper format
        if (glTFMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            int texIndex = glTFMaterial.pbrMetallicRoughness.baseColorTexture.index;
            GLuint textureID = loadTexture(gltfModel.images[gltfModel.textures[texIndex].source]);
            material.textureMap["albedoMap"] = textureID;
            std::cout << "Loaded albedo texture: " << textureID << std::endl;
        }
        
        if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
            int texIndex = glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
            GLuint textureID = loadTexture(gltfModel.images[gltfModel.textures[texIndex].source]);
            material.textureMap["metallicRoughnessMap"] = textureID;
            std::cout << "Loaded metallic-roughness texture: " << textureID << std::endl;
        }
    }
}

void Model::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    // Add error checking
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "OpenGL error after VBO setup: " << err << std::endl;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "OpenGL error after EBO setup: " << err << std::endl;
    }

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "OpenGL error after attribute setup: " << err << std::endl;
    }

    // Verify vertex data
    std::cout << "\nVertex Buffer Debug:" << std::endl;
    std::cout << "First triangle vertices:" << std::endl;
    for(int i = 0; i < 3; i++) {
        int idx = indices[i];
        std::cout << "v" << i << ": ("
                  << vertices[idx * 8 + 0] << ", "
                  << vertices[idx * 8 + 1] << ", "
                  << vertices[idx * 8 + 2] << ")" << std::endl;
    }

    glBindVertexArray(0);
}

GLuint Model::loadTexture(const tinygltf::Image& image) {
    std::cout << "\nLoading texture:" << std::endl;
    std::cout << "Width: " << image.width << std::endl;
    std::cout << "Height: " << image.height << std::endl;
    std::cout << "Components: " << image.component << std::endl;
    std::cout << "Data size: " << image.image.size() << std::endl;

    if (image.width == 0 || image.height == 0 || image.image.empty()) {
        std::cout << "Invalid image data!" << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Enable anisotropic filtering if available
    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    if (maxAniso > 0.0f) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    }

    // Set improved texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine format based on components
    GLint internalFormat = GL_RGBA8;
    GLenum format = GL_RGBA;
    if (image.component == 3) {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0,
                 format, GL_UNSIGNED_BYTE, image.image.data());

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "OpenGL error after texture upload: 0x" << std::hex << err << std::dec << std::endl;
    }

    // Verify the texture was created correctly
    GLint actualWidth, actualHeight, actualFormat;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &actualWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &actualHeight);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &actualFormat);

    std::cout << "Texture verification:" << std::endl;
    std::cout << "Actual dimensions: " << actualWidth << "x" << actualHeight << std::endl;
    std::cout << "Actual format: " << actualFormat << std::endl;
    std::cout << "Texture ID: " << textureID << std::endl;

    glGenerateMipmap(GL_TEXTURE_2D);
    return textureID;
}

void Model::Draw(Shader &shader) {
    shader.use();
    
    // Handle double-sided rendering
    if (material.doubleSided) {
        glDisable(GL_CULL_FACE);
        
        // Enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Adjust depth settings for transparent objects
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);  // Don't write to depth buffer for transparent objects
        glDepthFunc(GL_LESS);   // Still test against depth buffer
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    
    // Set material properties
    shader.setVec4("baseColorFactor", material.baseColorFactor);
    shader.setFloat("metallicFactor", material.metallicFactor);
    shader.setFloat("roughnessFactor", material.roughnessFactor);
    
    // Bind textures
    GLuint textureUnit = 0;
    
    // Bind albedo texture
    auto albedoIt = material.textureMap.find("albedoMap");
    if (albedoIt != material.textureMap.end()) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, albedoIt->second);
        shader.setInt("albedoMap", textureUnit++);
    }
    
    // Bind metallic-roughness texture
    auto mrIt = material.textureMap.find("metallicRoughnessMap");
    if (mrIt != material.textureMap.end()) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, mrIt->second);
        shader.setInt("metallicRoughnessMap", textureUnit++);
    }
    
    // Draw mesh
    glBindVertexArray(VAO);
    
    if (material.doubleSided) {
        // Draw back faces first
        glCullFace(GL_FRONT);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        
        // Then draw front faces
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    glBindVertexArray(0);
    
    // Restore default state
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
} 