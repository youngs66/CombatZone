#include "background.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"
#include <iostream>

Background::Background(const char* path) : 
    model(path),
    shader("shaders/gltf.vert", "shaders/gltf.frag")  // Use glTF shader
{
    scale = 25.0f;  // Reduce scale to 0.25
    std::cout << "Background shader program ID: " << shader.ID << std::endl;
}

void Background::Draw(const Camera &camera) {
    // Get window dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float aspectRatio = static_cast<float>(viewport[2]) / viewport[3];
    
    // Create view and projection matrices
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    
    // Set up shader
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", camera.GetPosition());
    
    // Create model matrix that follows camera
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, camera.GetPosition());
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    shader.setMat4("model", modelMatrix);
    
    // Save current OpenGL state
    GLboolean depthTest, blend, cullFace;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glGetBooleanv(GL_BLEND, &blend);
    glGetBooleanv(GL_CULL_FACE, &cullFace);
    
    // Set rendering state for background
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Change depth function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    
    // Draw the background
    model.Draw(shader);
    
    // Restore previous OpenGL state
    if (!depthTest) glDisable(GL_DEPTH_TEST);
    if (!blend) glDisable(GL_BLEND);
    if (!cullFace) glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);  // Restore default depth function
} 