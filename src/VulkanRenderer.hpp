#pragma once

#include "Vulkan.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct VulkanRendererSpecification
    {
        int maxFrame;
    };

    struct DrawTriangleInfo
    {
        VkCommandBuffer commandBuffer;
        VkRenderPass renderpass;
        std::vector<VkFramebuffer> frameBuffers;
        VkPipeline pipeline;
        VkExtent2D extent;
        VkSwapchainKHR swapchain;
        int frameIndex;
    };

    class VulkanRenderer
    {
    public:
        VulkanRenderer(const VulkanRendererSpecification& spec);
        ~VulkanRenderer();

        void DrawTriangle(const DrawTriangleInfo& info);

    private:
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkSemaphore> m_renderFinishedSPs;
        std::vector<VkSemaphore> m_imgAvailableSPs;
    };
}
