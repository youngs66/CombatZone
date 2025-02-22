#pragma once
#include "model.h"
#include "shader.h"
#include "../external/glm/glm/glm.hpp"
#include "../external/glm/glm/gtc/matrix_transform.hpp"

class Entity {
public:
    Entity(Model* model, Shader* shader, const glm::vec3& position = glm::vec3(0.0f),
           const glm::vec3& rotation = glm::vec3(0.0f),
           const glm::vec3& scale = glm::vec3(1.0f));

    void Draw();
    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::vec3& rotation);
    void SetScale(const glm::vec3& scale);
    
    glm::mat4 GetModelMatrix() const;
    Shader* GetShader() const { return shader; }

private:
    Model* model;
    Shader* shader;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    
    void UpdateModelMatrix();
}; 