#include "GL/Camera2D.h"

#include <iostream>

Camera2D::Camera2D(glm::vec2 position, int width, int height)
    : position(glm::vec3(position, 0.0f)), front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)),
      width(width), height(height)
{
}

glm::mat4 Camera2D::getViewMatrix() const
{
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

glm::mat4 Camera2D::getProjectionMatrix() const
{
    float halfWidth = this->width / 2.0f;
    float halfHeight = this->height / 2.0f;
    const float nearPlane = -1.0f;
    const float farPlane = 1.0f;
    return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
}