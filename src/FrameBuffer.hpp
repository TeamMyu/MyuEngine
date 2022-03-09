#pragma once

#include "Vulkan.hpp"
#include "RenderPass.hpp"

namespace VulkanWrapper
{
    struct FrameBufferSpecification
    {
        VkDevice device;
        VkImageView imgView;
        RenderPass* renderpass;
        uint32_t width;
        uint32_t hegiht;
    };

    class FrameBuffer
    {
    public:
        FrameBuffer(const FrameBufferSpecification& spec);
        ~FrameBuffer();
        VkFramebuffer GetVulkanFrameBuffer() {return m_VulkanFrameBuffer;}

    private:
        VkFramebuffer m_VulkanFrameBuffer;
        FrameBufferSpecification m_Specification;
    };
}
