#pragma once

#include "../VulkanWrapper/VulkanDevice.hpp"

namespace Myu
{
    class Renderer
    {
    public:
        Renderer(VulkanWrapper::VulkanDevice& vulkanDevice);

        void BeginDraw();
        void EndDraw();
        VkCommandBuffer GetCurrentBuffer() { return m_rVulkanDevice.GetCommandBuffers()[currentFrame]; }
        uint32_t        currentFrame = 0;

    private:
        // Class Ref
        VulkanWrapper::VulkanDevice& m_rVulkanDevice;
        // ---
        
    };
}  // namespace Myu::VulkanWrapper
