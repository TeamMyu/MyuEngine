#include "VulkanRenderPass.hpp"
#include "VulkanInstance.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    VulkanRenderPass::VulkanRenderPass(const VulkanRenderPassSpecification& spec)
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo VulkanRenderPassInfo{};
        VulkanRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        VulkanRenderPassInfo.attachmentCount = spec.attachments.size();
        VulkanRenderPassInfo.pAttachments = spec.attachments.data();
        VulkanRenderPassInfo.subpassCount = spec.subpasses.size();
        VulkanRenderPassInfo.pSubpasses = spec.subpasses.data();
        VulkanRenderPassInfo.dependencyCount = 1;
        VulkanRenderPassInfo.pDependencies = &dependency;

        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        if (vkCreateRenderPass(device, &VulkanRenderPassInfo, nullptr, &m_VkVulkanRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        vkDestroyRenderPass(device, m_VkVulkanRenderPass, nullptr);
    }
}
