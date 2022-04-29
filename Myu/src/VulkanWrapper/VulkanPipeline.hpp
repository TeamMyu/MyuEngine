#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

namespace Myu::VulkanWrapper
{
    struct VulkanPipelineSpecification
    {
        VulkanPipelineSpecification(const VulkanPipelineSpecification &) = delete;
        VulkanPipelineSpecification &operator=(const VulkanPipelineSpecification &) = delete;

        std::string vertFilepath;
        std::string fragFilepath;

        std::vector<VkVertexInputBindingDescription>   bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo              viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo         inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo         rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo           multisampleInfo{};
        VkPipelineColorBlendAttachmentState            colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo            colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo          depthStencilInfo{};
        std::vector<VkDynamicState>                    dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo               dynamicStateInfo{};
        VkPipelineLayout                               pipelineLayout = nullptr;

        VulkanPipelineSpecification();
    };

    class VulkanPipeline
    {
    public:
        VulkanPipeline(VulkanDevice &               vulkanDevice,
                       VkRenderPass                 vkRenderPass,
                       const VulkanPipelineSpecification &VulkanPipelineSpecification);
        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline &) = delete;
        VulkanPipeline operator=(const VulkanPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        VkShaderModule   createShaderModule(const VkDevice &device, const std::vector<char> &code);
        VkPipeline       GetVulkanPipeline() { return m_VkPipeline; }

    private:
        // Class Ref
        VulkanDevice     &m_rVulkanDevice;
        // ---
        VkPipeline m_VkPipeline;
    };
}  // namespace Myu::VulkanWrapper
