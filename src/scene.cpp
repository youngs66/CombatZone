#include "scene.h"
#include <iostream>

Scene::Scene() : aspectRatio(800.0f/600.0f) {
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
}

void Scene::AddShader(const std::string& name, const char* vertPath, const char* fragPath) {
    std::cout << "Adding shader: " << name << " from: " << vertPath << " and " << fragPath << std::endl;
    shaders[name] = std::make_unique<Shader>(vertPath, fragPath);
}

void Scene::AddModel(const std::string& name, const char* path) {
    std::cout << "Adding model: " << name << " from path: " << path << std::endl;
    models[name] = std::make_unique<Model>(path);
}

Entity* Scene::CreateEntity(const std::string& modelName, const std::string& shaderName,
                          const glm::vec3& position, const glm::vec3& rotation,
                          const glm::vec3& scale) {
    std::cout << "Creating entity with model: " << modelName << " and shader: " << shaderName << std::endl;
    
    auto model = models.find(modelName);
    auto shader = shaders.find(shaderName);
    
    if (model == models.end()) {
        std::cout << "Error: Model '" << modelName << "' not found!" << std::endl;
        return nullptr;
    }
    if (shader == shaders.end()) {
        std::cout << "Error: Shader '" << shaderName << "' not found!" << std::endl;
        return nullptr;
    }
    
    entities.push_back(std::make_unique<Entity>(
        model->second.get(),
        shader->second.get(),
        position, rotation, scale
    ));
    return entities.back().get();
}

void Scene::Update(float deltaTime) {
    // Update scene logic here
}

void Scene::Draw(const Camera& camera) {
    glm::mat4 view = camera.GetViewMatrix();
    glm::vec3 cameraPos = camera.GetPosition();
    
    // Update viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    aspectRatio = static_cast<float>(viewport[2]) / viewport[3];
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
    
    // Draw background entities first with special depth settings
    glDepthMask(GL_FALSE);  // Don't write to depth buffer
    for (const auto& entity : entities) {
        auto shader = entity->GetShader();
        if (shader->ID == shaders["background"]->ID) {
            // Update background position to match camera
            entity->SetPosition(cameraPos);
            
            shader->use();
            shader->setMat4("projection", projection);
            shader->setMat4("view", view);
            shader->setVec3("viewPos", cameraPos);
            entity->Draw();
        }
    }
    glDepthMask(GL_TRUE);  // Re-enable depth writing
    
    // Then draw other entities
    for (const auto& entity : entities) {
        auto shader = entity->GetShader();
        if (shader->ID != shaders["background"]->ID) {
            shader->use();
            shader->setMat4("projection", projection);
            shader->setMat4("view", view);
            shader->setVec3("viewPos", cameraPos);
            entity->Draw();
        }
    }
} 