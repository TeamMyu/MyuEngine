#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Camera2D
{
public:
    Camera2D(glm::vec2 position, int width, int height);
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    int width;
    int height;
};