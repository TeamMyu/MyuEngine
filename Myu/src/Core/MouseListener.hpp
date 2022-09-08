#pragma once

#include "../Vendor.hpp"
#include "GameObject.hpp"
#include "Camera.hpp"

namespace Myu
{
    class MouseListener
    {
    public:
        void moveInPlaneXZ(GLFWwindow* window, float dt, Camera& cm);
        double xPos{ 0.f }, yPos{ 0.f };
        double xCur{ 0.f }, yCur{ 0.f };
        float xOfs{ 0.f }, yOfs{ 0.f };

        glm::vec3 rotation{ 0.0f };
        glm::mat4 early_view{ 0.0f };

        float moveSpeed = 1.0f;
    };
}
