#include "VulkanRenderer.hpp"
#include "VulkanInstance.hpp"

namespace VulkanWrapper
{
    VulkanRenderer::VulkanRenderer(const VulkanRendererSpecification &spec)
    {
        m_imgAvailableSPs.resize(spec.maxFrame);
        m_renderFinishedSPs.resize(spec.maxFrame);
        m_InFlightFences.resize(spec.maxFrame);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        for (size_t i = 0; i < spec.maxFrame; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imgAvailableSPs[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSPs[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        for (size_t i = 0; i < m_InFlightFences.size(); i++) {
            vkDestroySemaphore(device, m_renderFinishedSPs[i], nullptr);
            vkDestroySemaphore(device, m_imgAvailableSPs[i], nullptr);
            vkDestroyFence(device, m_InFlightFences[i], nullptr);
        }
    }

    void VulkanRenderer::DrawTriangle(const DrawTriangleInfo &info)
    {
        // wait previous frame
        auto frameIndex = info.frameIndex;
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        vkWaitForFences(device, 1, &m_InFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &m_InFlightFences[frameIndex]);

        // Get Image from Swap chain
        uint32_t imgIndex;
        vkAcquireNextImageKHR(device, info.swapchain, UINT64_MAX, m_imgAvailableSPs[frameIndex], VK_NULL_HANDLE, &imgIndex);

        // clear data in Command Buffer
        vkResetCommandBuffer(info.commandBuffer, 0);

        // Start recording command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional
        if (vkBeginCommandBuffer(info.commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = info.renderpass;
        renderPassInfo.framebuffer = info.frameBuffers[imgIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = info.extent;

        // set screen clear color
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        
        vkCmdBeginRenderPass(info.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(info.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);

            vkCmdDraw(info.commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(info.commandBuffer);

        // End recording command buffer
        if (vkEndCommandBuffer(info.commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imgAvailableSPs[frameIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &info.commandBuffer;

        VkSemaphore signalSemaphores[] = {m_renderFinishedSPs[frameIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        auto graphicsQueue = VulkanInstance::instance().m_Device->GetVkGraphicsQueue();
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_InFlightFences[frameIndex]) != VK_SUCCESS) {
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
        
        auto presentQueue = VulkanInstance::instance().m_Device->GetVkPresentQueue();
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }
}
