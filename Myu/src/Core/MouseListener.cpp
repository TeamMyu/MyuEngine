#include "MouseListener.hpp"

namespace Myu
{
    void MouseListener::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& go) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
            (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)) {

            double xCur, yCur;
            glfwGetCursorPos(window, &xCur, &yCur);

            if (xPos == 0 && yPos == 0) {
                xPos = xCur;
                yPos = yCur;
                pre_rotation = go.transform.rotation;
            }

            double xOfs, yOfs;
            xOfs = (xCur - xPos) * moveSpeed;
            yOfs = (yPos - yCur) * moveSpeed;

            glm::vec3 rotate{ 0 };
            rotate.x = yOfs / 1280 * moveSpeed;
            rotate.y = xOfs / 800 * moveSpeed;

            if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
                go.transform.rotation = pre_rotation + (moveSpeed * rotate);

            go.transform.rotation.x = pre_rotation.x + glm::clamp(rotate.x, -1.5f, 1.5f); // -85 ~ 85 degree
            go.transform.rotation.y = pre_rotation.y + glm::mod(rotate.y, glm::two_pi<float>());

        } else {
            xPos = 0;
            yPos = 0;
        }
    }
}
