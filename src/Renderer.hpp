#pragma once

#include "Vulkan.hpp"
#include "RenderPass.hpp"
#include "CommandBuffers.hpp"
#include "FrameBuffer.hpp"
#include "GraphicsPipeline.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct RendererSpecification
    {
        VkDevice device;
        int maxFrame;
    };

    struct DrawTriangleInfo
    {
        CommandBuffers* commandBuffers;
        RenderPass* renderpass;
        FrameBuffer* frameBuffer;
        GraphicsPipeline* pipeline;
        VkExtent2D extent;
        VkSwapchainKHR swapchain;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        int frameIndex;
    };

    class Renderer
    {
    public:
        Renderer(const RendererSpecification& spec);
        ~Renderer();

        void DrawTriangle(const DrawTriangleInfo& info);

    private:
        RendererSpecification m_Specfication;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkSemaphore> m_renderFinishedSPs;
        std::vector<VkSemaphore> m_imgAvailableSPs;
    };
}
