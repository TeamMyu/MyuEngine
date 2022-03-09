#pragma once

#include "Vulkan.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct PipelineSpecification
    {
        VkDevice device;
        RenderPass* renderpass;
        VkExtent2D extent;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(const PipelineSpecification &spec);
        ~GraphicsPipeline();
        VkShaderModule createShaderModule(const VkDevice &device, const std::vector<char> &code);
        VkPipeline GetVulkanPipeline() { return m_VkPipeline; }
        VkPipelineLayout GetVulkanPipelineLayout() {return m_VkpipelineLayout;}

    private:
        VkPipeline m_VkPipeline;
        VkPipelineLayout m_VkpipelineLayout;
        PipelineSpecification m_Specification;
    };
}
