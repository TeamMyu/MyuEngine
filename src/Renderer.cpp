#include "Renderer.hpp"

namespace VulkanWrapper
{
    Renderer::Renderer(const RendererSpecification &spec)
        : m_Specfication(spec)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto device = spec.device;
        for (int i = 0; i < spec.maxFrame; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imgAvailableSPs[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSPs[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    Renderer::~Renderer()
    {

    }

    void Renderer::DrawTriangle(const DrawTriangleInfo &info)
    {
        // wait previous frame
        auto frameIndex = info.frameIndex;
        auto device = m_Specfication.device;
        vkWaitForFences(device, 1, &m_InFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &m_InFlightFences[frameIndex]);

        // Get Image from Swap chain
        uint32_t imgIndex;
        vkAcquireNextImageKHR(device, info.swapchain, UINT64_MAX, m_imgAvailableSPs[frameIndex], VK_NULL_HANDLE, &imgIndex);

        // clear data in Command Buffer
        auto commandBuffer = info.commandBuffers[frameIndex];
        vkResetCommandBuffer(commandBuffer.GetVkCommandBuffer(frameIndex), 0);

        // Start recording command buffer
        // TOOD: remove frameIndex parameter
        commandBuffer.Begin(frameIndex);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = info.renderpass->GetVulkanRenderPass();
        renderPassInfo.framebuffer = info.frameBuffer->GetVulkanFrameBuffer();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = info.extent;

        // set screen clear color
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        auto vkCommandBuffer = commandBuffer.GetVkCommandBuffer(frameIndex);
        auto pipeline = info.pipeline->GetVulkanPipeline();
        vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            vkCmdDraw(vkCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(vkCommandBuffer);

        // End recording command buffer
        commandBuffer.End(frameIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imgAvailableSPs[frameIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;

        VkSemaphore signalSemaphores[] = {m_renderFinishedSPs[frameIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(info.graphicsQueue, 1, &submitInfo, m_InFlightFences[frameIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        VkSwapchainKHR swapChains[] = {info.swapchain};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imgIndex;

        vkQueuePresentKHR(info.presentQueue, &presentInfo);
    }
}
