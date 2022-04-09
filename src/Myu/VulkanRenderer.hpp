#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "Window.hpp"

namespace VulkanWrapper
{
    class VulkanRenderer
    {
    public:
        VulkanRenderer(VulkanDevice& vulkanDevice, MyuEngine::Window& window);

        void BeginDraw();
        void EndDraw();
        VkCommandBuffer GetCurrentBuffer() { return m_rVulkanDevice.GetCommandBuffers()[currentFrame]; }
        uint32_t        currentFrame = 0;

    private:
        // Class Ref
        VulkanDevice& m_rVulkanDevice;
        MyuEngine::Window& m_rWindow;
        // ---
        
    };
}  // namespace VulkanWrapper