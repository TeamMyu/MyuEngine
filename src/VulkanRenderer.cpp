#include "VulkanRenderer.hpp"

namespace VulkanWrapper
{
    VulkanRenderer::VulkanRenderer(VulkanDevice& vulkanDevice, MyuEngine::Window& window)
        : m_rVulkanDevice{vulkanDevice}
        , m_rWindow{window}
    {
    }
    void VulkanRenderer::BeginDraw()
    {
        //auto currentBuffer = GetCurrentBuffer();
     
    }

    void VulkanRenderer::EndDraw()
    {
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}  // namespace VulkanWrapper