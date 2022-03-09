#include "VulkanFrameBuffer.hpp"
#include "VulkanInstance.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    VulkanFrameBuffer::VulkanFrameBuffer(const VulkanFrameBufferSpecification &spec)
    {
        VkImageView attachments[] = {spec.imgView};

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = spec.renderpass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = spec.extent.width;
        frameBufferInfo.height = spec.extent.height;
        frameBufferInfo.layers = 1;

        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr, &m_VulkanVulkanFrameBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create VulkanFrameBuffer!");
        }
    }

    // TODO: Dispose라는 명시된 함수로 구현하는게 나으려나?
    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        vkDestroyFramebuffer(device, m_VulkanVulkanFrameBuffer, nullptr);
    }
}
