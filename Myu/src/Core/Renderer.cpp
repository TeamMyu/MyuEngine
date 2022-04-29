#include "Renderer.hpp"

namespace Myu
{
    Renderer::Renderer(VulkanWrapper::VulkanDevice& vulkanDevice)
        : m_rVulkanDevice{vulkanDevice}
    {
    }

    void Renderer::BeginDraw()
    {
        auto currentBuffer = GetCurrentBuffer();
        VulkanWrapper::recordCommandBuffer(currentBuffer);
    }

    void Renderer::EndDraw()
    {
        currentFrame = (currentFrame + 1) % VulkanWrapper::MAX_FRAMES_IN_FLIGHT;
    }
}  // namespace Myu::VulkanWrapper
