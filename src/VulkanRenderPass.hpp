#pragma once

#include "Vulkan.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct VulkanRenderPassSpecification
    {
        VkFormat imgFormat;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkAttachmentDescription> attachments;
    };

    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(const VulkanRenderPassSpecification &spec);
        ~VulkanRenderPass();

        VkRenderPass GetVkRenderPass() { return m_VkVulkanRenderPass; }

    private:
        VkRenderPass m_VkVulkanRenderPass;
    };
}
