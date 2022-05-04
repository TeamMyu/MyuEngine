#pragma once

#include "VulkanDevice.hpp"

namespace Myu::VulkanWrapper
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(VulkanDevice& vulkanDevice, VkExtent2D extent);
        VulkanSwapchain(VulkanDevice&                    vulkanDevice,
                        VkExtent2D                       extent,
                        std::shared_ptr<VulkanSwapchain> oldSwapChain);
        ~VulkanSwapchain();

        VulkanSwapchain(const VulkanSwapchain&) = delete;
        VulkanSwapchain operator=(const VulkanSwapchain&) = delete;

        VkSwapchainKHR             GetVkSwapChain() { return m_SwapChain; }
        std::vector<VkImage>       GetVkImages() { return m_SwapChainImages; }
        VkFormat                   GetVkFormat() { return m_SwapChainImageFormat; }
        VkExtent2D                 GetVkExtent2D() { return m_SwapChainExtent; }
        std::vector<VkImageView>   GetImageViews() { return m_ImageViews; }
        std::vector<VkFramebuffer> GetVkFrameBuffers() { return m_FrameBuffers; }
        VkRenderPass               GetVkRenderPass() { return m_RenderPass; }

        VkResult                   AcquireNextImage(uint32_t* imageIndex, uint32_t currentFrame);

        VkResult PresentQueue(std::vector<VkCommandBuffer>& currentBuffer, uint32_t imageIndex, uint32_t currentFrame);

        void BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void EndRenderPass(VkCommandBuffer commandBuffer);

        bool compareSwapFormats(const VulkanSwapchain& swapChain) const
        {
            return swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

    private:
        // Class Ref
        VulkanDevice& m_rVulkanDevice;
        // ---
        VkSwapchainKHR             m_SwapChain;
        VkRenderPass               m_RenderPass;
        std::vector<VkImageView>   m_ImageViews;
        std::vector<VkFramebuffer> m_FrameBuffers;
        std::vector<VkImage>       m_SwapChainImages;
        VkFormat                   m_SwapChainImageFormat;
        VkExtent2D                 m_SwapChainExtent;

        VkImage        m_DepthImage;
        VkDeviceMemory m_DepthImageMemory;
        VkImageView    m_DepthImageView;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence>     inFlightFences;

        void createSwapchain();
        void createImageViews();
        void createFrameBuffers();
        void createRenderPass();
        void createDepthResources();
        void createSyncObjects();

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR   chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D         chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };
}  // namespace Myu::VulkanWrapper
