#pragma once

#include "../Vendor.hpp"
#include "GameObject.hpp"

namespace Myu
{
    class MouseListener
    {
    public:
        void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& go);
        double xPos = 0, yPos = 0;

        glm::vec3 pre_rotation{ 0.0f };

        float moveSpeed = 1.5f;
    };
}
