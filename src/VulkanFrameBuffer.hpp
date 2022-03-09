#pragma once

#include "Vulkan.hpp"

namespace VulkanWrapper
{
    struct VulkanFrameBufferSpecification
    {
        VkImageView imgView;
        VkRenderPass renderpass;
        VkExtent2D extent;
    };

    class VulkanFrameBuffer
    {
    public:
        VulkanFrameBuffer(const VulkanFrameBufferSpecification& spec);
        ~VulkanFrameBuffer();
        VkFramebuffer GetVkFrameBuffer() {return m_VulkanVulkanFrameBuffer;}

    private:
        VkFramebuffer m_VulkanVulkanFrameBuffer;
    };
}
