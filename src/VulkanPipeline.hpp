#pragma once

#include "Vulkan.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct PipelineSpecification
    {
        VkRenderPass renderpass;
        VkExtent2D extent;
    };

    class VulkanPipeline
    {
    public:
        VulkanPipeline(const PipelineSpecification &spec);
        ~VulkanPipeline();

        VkShaderModule createShaderModule(const VkDevice &device, const std::vector<char> &code);
        VkPipeline GetVulkanPipeline() { return m_VkPipeline; }
        VkPipelineLayout GetVulkanPipelineLayout() {return m_VkpipelineLayout;}

    private:
        VkPipeline m_VkPipeline;
        VkPipelineLayout m_VkpipelineLayout;
    };
}
