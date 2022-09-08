#include "MouseListener.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Myu
{
    void MouseListener::moveInPlaneXZ(GLFWwindow* window, float dt, Camera& cm) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
            (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)) {

            glfwGetCursorPos(window, &xCur, &yCur);

            if (xPos < glm::epsilon<double>() && yPos < glm::epsilon<double>()) {
                xPos = xCur;
                yPos = yCur;
                if (early_view == glm::mat4(0.0f))
                    early_view = cm.getView();
            }
          
            xOfs = (xCur - xPos) * moveSpeed;
            yOfs = (yPos - yCur) * moveSpeed;
            glm::mat4 Rotation = glm::rotate(early_view, glm::radians(rotation.y + yOfs), { 1, 0, 0 });
            Rotation = glm::rotate(Rotation, glm::radians(rotation.x + xOfs), { 0, 1, 0 });
            cm.setView(Rotation);

        } else if (xPos > glm::epsilon<double>() && yPos > glm::epsilon<double>()) {
                rotation.x += xOfs;
                rotation.y += yOfs;
                xPos = 0.f;
                yPos = 0.f;
        }
    }
}
