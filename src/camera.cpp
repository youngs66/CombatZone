#include "camera.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"

Camera::Camera(glm::vec3 position)
    : Position(position),
      Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      Up(glm::vec3(0.0f, 1.0f, 0.0f)),
      Right(glm::vec3(1.0f, 0.0f, 0.0f)),
      WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      Yaw(-90.0f),
      Pitch(0.0f),
      MovementSpeed(10.0f),
      MouseSensitivity(0.1f) {
    updateCameraVectors();
}

void Camera::ProcessKeyboard(char direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    float rotationSpeed = 90.0f * deltaTime;  // 90 degrees per second
    
    switch(direction) {
        case 'W':
            Position += Front * velocity;  // Move forward
            break;
        case 'S':
            Position -= Front * velocity;  // Move backward
            break;
        case 'A':
            Yaw -= rotationSpeed;  // Rotate left
            updateCameraVectors();
            break;
        case 'D':
            Yaw += rotationSpeed;  // Rotate right
            updateCameraVectors();
            break;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

glm::vec3 Camera::GetPosition() const {
    return Position;
} 