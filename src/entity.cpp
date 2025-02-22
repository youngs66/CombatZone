#include "entity.h"

Entity::Entity(Model* model, Shader* shader, const glm::vec3& position,
               const glm::vec3& rotation, const glm::vec3& scale)
    : model(model), shader(shader), position(position), rotation(rotation), scale(scale) {
    UpdateModelMatrix();
}

void Entity::Draw() {
    shader->use();
    shader->setMat4("model", modelMatrix);
    model->Draw(*shader);
}

void Entity::SetPosition(const glm::vec3& newPosition) {
    position = newPosition;
    UpdateModelMatrix();
}

void Entity::SetRotation(const glm::vec3& newRotation) {
    rotation = newRotation;
    UpdateModelMatrix();
}

void Entity::SetScale(const glm::vec3& newScale) {
    scale = newScale;
    UpdateModelMatrix();
}

glm::mat4 Entity::GetModelMatrix() const {
    return modelMatrix;
}

void Entity::UpdateModelMatrix() {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);
} 