#pragma once

#include "VulkanDevice.hpp"

namespace VulkanWrapper
{
    class VulkanCamera
    {
    public:
        VulkanCamera();
        ~VulkanCamera();

    private:

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjMatrix;
    };
}  // namespace VulkanWrapper