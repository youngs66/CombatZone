#pragma once
#include "entity.h"
#include "camera.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class Scene {
public:
    Scene();
    
    // Resource management
    void AddShader(const std::string& name, const char* vertPath, const char* fragPath);
    void AddModel(const std::string& name, const char* path);
    
    // Entity management
    Entity* CreateEntity(const std::string& modelName, const std::string& shaderName,
                        const glm::vec3& position = glm::vec3(0.0f),
                        const glm::vec3& rotation = glm::vec3(0.0f),
                        const glm::vec3& scale = glm::vec3(1.0f));
    
    void Update(float deltaTime);
    void Draw(const Camera& camera);

private:
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::vector<std::unique_ptr<Entity>> entities;
    
    glm::mat4 projection;
    float aspectRatio;
}; 