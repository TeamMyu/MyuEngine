
#include "VulkanPipeline.hpp"
#include "../Core/Utils.hpp"

namespace Myu::VulkanWrapper
{
    VulkanPipeline::VulkanPipeline(VulkanDevice&                      vulkanDevice,
                                   VkRenderPass                       vkRenderPass,
                                   const VulkanPipelineSpecification& VulkanPipelineSpecification)
        : m_rVulkanDevice{vulkanDevice}
    {
        if (VulkanPipelineSpecification.pipelineType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
        {
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType               = VulkanPipelineSpecification.pipelineType;
            pipelineInfo.stageCount          = VulkanPipelineSpecification.shaderStages.size();
            pipelineInfo.pStages             = VulkanPipelineSpecification.shaderStages.data();
            pipelineInfo.pVertexInputState   = &VulkanPipelineSpecification.vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &VulkanPipelineSpecification.inputAssemblyInfo;
            pipelineInfo.pViewportState      = &VulkanPipelineSpecification.viewportInfo;
            pipelineInfo.pRasterizationState = &VulkanPipelineSpecification.rasterizationInfo;
            pipelineInfo.pMultisampleState   = &VulkanPipelineSpecification.multisampleInfo;
            pipelineInfo.pColorBlendState    = &VulkanPipelineSpecification.colorBlendInfo;
            pipelineInfo.pDepthStencilState  = &VulkanPipelineSpecification.depthStencilInfo;
            pipelineInfo.layout              = VulkanPipelineSpecification.pipelineLayout;
            pipelineInfo.renderPass          = vkRenderPass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(m_rVulkanDevice.GetVkLogicalDevice(),
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &m_VkPipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create graphics pipeline!");
            }
        }
        else if (VulkanPipelineSpecification.pipelineType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
        {
            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.basePipelineIndex  = 0;
            pipelineInfo.layout              = VulkanPipelineSpecification.pipelineLayout;
            pipelineInfo.pNext              = nullptr;
            pipelineInfo.stage              = VulkanPipelineSpecification.shaderStages[0];
            pipelineInfo.sType               = VulkanPipelineSpecification.pipelineType;

            if (vkCreateComputePipelines(m_rVulkanDevice.GetVkLogicalDevice(),
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &m_VkPipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create graphics pipeline!");
            }
        }

    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_rVulkanDevice.GetVkLogicalDevice(), m_VkPipeline, nullptr);
    }

    void VulkanPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipeline);
    }

    VulkanPipelineSpecification::VulkanPipelineSpecification()
    {
        this->pipelineType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        this->vertexInputInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        inputAssemblyInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        this->inputAssemblyInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        this->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        this->viewportInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        this->viewportInfo.viewportCount = 1;
        this->viewportInfo.pViewports    = nullptr;
        this->viewportInfo.scissorCount  = 1;
        this->viewportInfo.pScissors     = nullptr;

        this->rasterizationInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        this->rasterizationInfo.depthClampEnable        = VK_FALSE;
        this->rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        this->rasterizationInfo.polygonMode             = VK_POLYGON_MODE_FILL;
        this->rasterizationInfo.lineWidth               = 1.0f;
        this->rasterizationInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
        this->rasterizationInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        this->rasterizationInfo.depthBiasEnable         = VK_FALSE;
        this->rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        this->rasterizationInfo.depthBiasClamp          = 0.0f;  // Optional
        this->rasterizationInfo.depthBiasSlopeFactor    = 0.0f;  // Optional

        this->multisampleInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        this->multisampleInfo.sampleShadingEnable   = VK_FALSE;
        this->multisampleInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        this->multisampleInfo.minSampleShading      = 1.0f;      // Optional
        this->multisampleInfo.pSampleMask           = nullptr;   // Optional
        this->multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        this->multisampleInfo.alphaToOneEnable      = VK_FALSE;  // Optional

        this->colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        this->colorBlendAttachment.blendEnable = VK_TRUE;
        this->colorBlendAttachment.srcColorBlendFactor =
            VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
        this->colorBlendAttachment.dstColorBlendFactor =
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;                                   // Optional
        this->colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;  // Optional
        this->colorBlendAttachment.srcAlphaBlendFactor =
            VK_BLEND_FACTOR_ONE;  // Optional
        this->colorBlendAttachment.dstAlphaBlendFactor =
            VK_BLEND_FACTOR_ZERO;                                   // Optional
        this->colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;  // Optional

        this->colorBlendInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        this->colorBlendInfo.logicOpEnable   = VK_FALSE;
        this->colorBlendInfo.logicOp         = VK_LOGIC_OP_COPY;  // Optional
        this->colorBlendInfo.attachmentCount = 1;
        this->colorBlendInfo.pAttachments =
            &(this->colorBlendAttachment);
        this->colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        this->colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        this->colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        this->colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        this->depthStencilInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        this->depthStencilInfo.depthTestEnable       = VK_TRUE;
        this->depthStencilInfo.depthWriteEnable      = VK_TRUE;
        this->depthStencilInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
        this->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        this->depthStencilInfo.minDepthBounds        = 0.0f;  // Optional
        this->depthStencilInfo.maxDepthBounds        = 1.0f;  // Optional
        this->depthStencilInfo.stencilTestEnable     = VK_FALSE;
        this->depthStencilInfo.front                 = {};  // Optional
        this->depthStencilInfo.back                  = {};  // Optional
    }
}  // namespace Myu::VulkanWrapper
