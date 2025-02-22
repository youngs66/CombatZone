#pragma once
#include "../external/glm/glm/glm.hpp"  // Use angle brackets for GLM
#include "../external/glm/glm/gtc/matrix_transform.hpp"

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
    
    glm::mat4 GetViewMatrix() const;
    void ProcessKeyboard(char direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    
    glm::vec3 GetPosition() const;

private:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    float MovementSpeed;
    float MouseSensitivity;
    float Yaw;
    float Pitch;
    
    void updateCameraVectors();
}; 