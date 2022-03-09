#pragma once

#include "Vulkan.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct RenderPassSpecification
    {
        VkDevice device;
        VkFormat imgFormat;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkAttachmentDescription> attachments;
    };

    class RenderPass
    {
    public:
        RenderPass(const RenderPassSpecification &spec);
        VkRenderPass GetVulkanRenderPass() { return m_VulkanRenderPass; }

    private:
        VkRenderPass m_VulkanRenderPass;
    };
}
