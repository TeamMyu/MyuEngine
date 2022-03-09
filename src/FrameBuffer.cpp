#include "FrameBuffer.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    FrameBuffer::FrameBuffer(const FrameBufferSpecification &spec)
        : m_Specification(spec)
    {
        VkImageView attachments[] = {spec.imgView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = spec.renderpass->GetVulkanRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = spec.width;
        framebufferInfo.height = spec.hegiht;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(spec.device, &framebufferInfo, nullptr, &m_VulkanFrameBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    // TODO: Dispose라는 명시된 함수로 구현하는게 나으려나?
    FrameBuffer::~FrameBuffer()
    {
        vkDestroyFramebuffer(m_Specification.device, m_VulkanFrameBuffer, nullptr);
    }
}
