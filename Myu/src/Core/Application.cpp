#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"

#include <chrono>
#include "../VulkanWrapper/VulkanTexture.hpp"
#include "../VulkanWrapper/VulkanInitializer.hpp"

#include <glm/gtc/type_ptr.hpp>

VkPipelineLayout pipelineLayout;

VkPipeline graphicsPipeline;

struct Compute
{
    VkQueue                                          queue;                // Separate queue for compute commands (queue family may differ from the one used for graphics)
    VkCommandPool                                    commandPool;          // Use a separate command pool (queue family may differ from the one used for graphics)
    VkCommandBuffer                                  commandBuffer;        // Command buffer storing the dispatch commands and barriers
    VkSemaphore                                      semaphore;            // Execution dependency between compute & graphic submission
    VkDescriptorSetLayout                            descriptorSetLayout;  // Compute shader binding layout
    VkDescriptorSet                                  descriptorSet;        // Compute shader bindings
    VkPipelineLayout                                 pipelineLayout;       // Layout of the compute pipeline
    std::vector<Myu::VulkanWrapper::VulkanPipeline*> pipelines;            // Compute pipelines for image filters
    int32_t                                          pipelineIndex = 0;    // Current image filtering compute pipeline index
    VkFence                                          fence;

    VkBuffer       uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    Myu::VulkanWrapper::UniformBufferObject ubo;
    Myu::VulkanWrapper::VulkanTexture       positionMap;
    Myu::VulkanWrapper::VulkanTexture       normalMap;
    Myu::VulkanWrapper::VulkanTexture       colorMap;
    Myu::VulkanWrapper::VulkanTexture       uvMap;
} compute;

struct
{
    VkDescriptorSetLayout descriptorSetLayout;  // Image display shader binding layout
    VkDescriptorSet       descriptorSet;
    //VkPipeline            pipeline;                  // Image display pipeline
    VkPipelineLayout                    pipelineLayout;  // Layout of the graphics pipeline
    VkSemaphore                         semaphore;       // Execution dependency between compute & graphic submission
    VkRenderPass                        renderPass;
    VkFramebuffer                       frameBuffer;
    Myu::VulkanWrapper::VulkanPipeline* pipeline;
    VkCommandBuffer                     commandBuffer;

    VkSampler frameSampler;

    Myu::VulkanWrapper::VulkanTexture position;
    Myu::VulkanWrapper::VulkanTexture normal;
    Myu::VulkanWrapper::VulkanTexture color;
    Myu::VulkanWrapper::VulkanTexture uv;
    Myu::VulkanWrapper::VulkanTexture depth;

    uint32_t width;
    uint32_t height;

} offscreen;

struct
{
    VkDescriptorSetLayout descriptorSetLayout;  // Compute shader binding layout
    VkDescriptorSet       descriptorSet;        // Compute shader bindings
    VkPipelineLayout      pipelineLayout;       // Layout of the compute pipeline
    VkSemaphore           semaphore;            // Execution dependency between compute & graphic submission
} graphics;

std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
Myu::KeyboardListener                                       keyboardListener{};
Myu::Camera                                                 camera = Myu::Camera();

namespace Myu
{
    VkFormat getSupportedDepthFormat(VkPhysicalDevice physicalDevice)
    {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<VkFormat> depthFormats = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM};

        for (auto& format : depthFormats)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
            // Format must support depth stencil attachment for optimal tiling
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                return format;
            }
        }

        return (VkFormat)-1;
    }

    Application::Application()
    {
        loadGameObjects();

        //FIXME: HACK
        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &gameObjects[0].model->getMeshes()[0].getMaterial().getDescriptorLayout(), &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));

        offscreen.width  = m_Swapchain.GetVkExtent2D().width;
        offscreen.height = m_Swapchain.GetVkExtent2D().height;

        // create texture relevent resources
        offscreen.position.mSpec.samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        offscreen.position.mSpec.imageUsageFlags       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        offscreen.position.mSpec.imageLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreen.position.createTextureTarget(&m_Device, offscreen.width, offscreen.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
        // muset set image size to be frame buffer size

        offscreen.normal.mSpec.samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        offscreen.normal.mSpec.imageUsageFlags       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        offscreen.normal.mSpec.imageLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreen.normal.createTextureTarget(&m_Device, offscreen.width, offscreen.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
        // muset set image size to be frame buffer size

        offscreen.color.mSpec.samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        offscreen.color.mSpec.imageUsageFlags       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        offscreen.color.mSpec.imageLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreen.color.createTextureTarget(&m_Device, offscreen.width, offscreen.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
        // muset set image size to be frame buffer size

        offscreen.uv.mSpec.samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        offscreen.uv.mSpec.imageUsageFlags       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        offscreen.uv.mSpec.imageLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreen.uv.createTextureTarget(&m_Device, offscreen.width, offscreen.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
        // muset set image size to be frame buffer size

        offscreen.depth.mSpec.samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        offscreen.depth.mSpec.imageUsageFlags       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        offscreen.depth.mSpec.imageLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        offscreen.depth.createTextureTarget(&m_Device, offscreen.width, offscreen.height, getSupportedDepthFormat(m_Device.GetVkPhysicalDevice()), (VkImageAspectFlagBits)(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
        // muset set image size to be frame buffer size

        std::array<VkAttachmentDescription, 5> attachmentDescs = {};

        // Init attachment properties
        for (uint32_t i = 0; i < 5; ++i)
        {
            attachmentDescs[i].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescs[i].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescs[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescs[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            if (i == 4)
            {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            else
            {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                //attachmentDescs[i].finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
        }

        // Formats
        attachmentDescs[0].format = offscreen.position.format;
        attachmentDescs[2].format = offscreen.normal.format;
        attachmentDescs[1].format = offscreen.color.format;
        attachmentDescs[3].format = offscreen.uv.format;
        attachmentDescs[4].format = offscreen.depth.format;

        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        VkAttachmentReference depthReference = {};
        depthReference.attachment            = 4;
        depthReference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass    = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments       = colorReferences.data();
        subpass.colorAttachmentCount    = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for attachment layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = 0;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pAttachments           = attachmentDescs.data();
        renderPassInfo.attachmentCount        = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 2;
        renderPassInfo.pDependencies          = dependencies.data();

        vkCreateRenderPass(m_Device.GetVkLogicalDevice(), &renderPassInfo, nullptr, &offscreen.renderPass);

        std::array<VkImageView, 5> attachments;
        attachments[0] = offscreen.position.mImageView;
        attachments[2] = offscreen.normal.mImageView;
        attachments[1] = offscreen.color.mImageView;
        attachments[3] = offscreen.uv.mImageView;
        attachments[4] = offscreen.depth.mImageView;

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.pNext                   = NULL;
        fbufCreateInfo.renderPass              = offscreen.renderPass;
        fbufCreateInfo.pAttachments            = attachments.data();
        fbufCreateInfo.attachmentCount         = static_cast<uint32_t>(attachments.size());
        fbufCreateInfo.width                   = offscreen.width;
        fbufCreateInfo.height                  = offscreen.height;
        fbufCreateInfo.layers                  = 1;
        vkCreateFramebuffer(m_Device.GetVkLogicalDevice(), &fbufCreateInfo, nullptr, &offscreen.frameBuffer);

        // Create sampler to sample from the color attachments
        VkSamplerCreateInfo sampler{};
        sampler.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.maxAnisotropy = 1.0f;
        sampler.magFilter     = VK_FILTER_NEAREST;
        sampler.minFilter     = VK_FILTER_NEAREST;
        sampler.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeV  = sampler.addressModeU;
        sampler.addressModeW  = sampler.addressModeU;
        sampler.mipLodBias    = 0.0f;
        sampler.maxAnisotropy = 1.0f;
        sampler.minLod        = 0.0f;
        sampler.maxLod        = 1.0f;
        sampler.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateSampler(m_Device.GetVkLogicalDevice(), &sampler, nullptr, &offscreen.frameSampler);

        // --

        VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};

        // viewport info setup
        VkViewport viewport{};
        viewport.x                           = 0.0f;
        viewport.y                           = 0.0f;
        viewport.width                       = (float)m_Swapchain.GetVkExtent2D().width;
        viewport.height                      = (float)m_Swapchain.GetVkExtent2D().height;
        viewport.minDepth                    = 0.0f;
        viewport.maxDepth                    = 1.0f;
        pipelineSpec.viewportInfo.pViewports = &viewport;

        VkRect2D scissor{};
        scissor.offset                      = {0, 0};
        scissor.extent                      = m_Swapchain.GetVkExtent2D();
        pipelineSpec.viewportInfo.pScissors = &scissor;

        {
            VkDescriptorSetLayoutBinding dslb{};
            dslb.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            dslb.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
            dslb.binding         = 0;
            dslb.descriptorCount = 1;

            std::vector<VkDescriptorSetLayoutBinding> dslbs = {dslb};

            VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
            descriptorLayoutCI.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorLayoutCI.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            descriptorLayoutCI.bindingCount = static_cast<uint32_t>(dslbs.size());
            descriptorLayoutCI.pBindings    = dslbs.data();

            vkCreateDescriptorSetLayout(m_Device.GetVkLogicalDevice(), &descriptorLayoutCI, nullptr, &graphics.descriptorSetLayout);  // 유니폼 하나

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts    = &graphics.descriptorSetLayout;

            vkCreatePipelineLayout(m_Device.GetVkLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout);

            auto vertShaderCode = Utils::readFile("shaders/combine.vert.spv");
            auto fragShaderCode = Utils::readFile("shaders/combine.frag.spv");

            VkShaderModule vertShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), vertShaderCode);
            VkShaderModule fragShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName  = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName  = "main";

            VkPipelineVertexInputStateCreateInfo emptyInputState{};
            emptyInputState.sType        = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineSpec.vertexInputInfo = emptyInputState;

            pipelineSpec.shaderStages.push_back(fragShaderStageInfo);
            pipelineSpec.shaderStages.push_back(vertShaderStageInfo);
            pipelineSpec.rasterizationInfo.cullMode         = VK_CULL_MODE_NONE;
            pipelineSpec.depthStencilInfo.stencilTestEnable = VK_TRUE;
            pipelineSpec.depthStencilInfo.depthWriteEnable  = VK_TRUE;
            pipelineSpec.depthStencilInfo.back.compareOp    = VK_COMPARE_OP_ALWAYS;
            pipelineSpec.depthStencilInfo.back.failOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.depthFailOp  = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.passOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.compareMask  = 0xff;
            pipelineSpec.depthStencilInfo.back.writeMask    = 0xff;
            pipelineSpec.depthStencilInfo.back.reference    = 1;
            pipelineSpec.depthStencilInfo.front             = pipelineSpec.depthStencilInfo.back;
            pipelineSpec.pipelineLayout                     = graphics.pipelineLayout;

            m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);  // 합쳐서 그리는애 -> 이미지 샘플러

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
        }

        {
            auto vertShaderCode = Utils::readFile("shaders/gbuf.vert.spv");
            auto fragShaderCode = Utils::readFile("shaders/gbuf.frag.spv");

            VkShaderModule vertShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), vertShaderCode);
            VkShaderModule fragShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName  = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName  = "main";

            std::vector<VkVertexInputBindingDescription>   bindingDescriptions{};
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

            attributeDescriptions = Myu::VulkanWrapper::Vertex::getAttributeDescriptions();
            bindingDescriptions   = Myu::VulkanWrapper::Vertex::getBindingDescription();

            VkPipelineVertexInputStateCreateInfo vertexInputState{};
            vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(Myu::VulkanWrapper::Vertex::getAttributeDescriptions().size());
            vertexInputState.vertexBindingDescriptionCount   = static_cast<uint32_t>(Myu::VulkanWrapper::Vertex::getBindingDescription().size());
            vertexInputState.pVertexAttributeDescriptions    = attributeDescriptions.data();
            vertexInputState.pVertexBindingDescriptions      = bindingDescriptions.data();

            pipelineSpec.vertexInputInfo = vertexInputState;

            pipelineSpec.shaderStages.clear();
            pipelineSpec.shaderStages.push_back(fragShaderStageInfo);
            pipelineSpec.shaderStages.push_back(vertShaderStageInfo);

            pipelineSpec.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

            VkPipelineColorBlendAttachmentState blendAttach1{};
            blendAttach1.colorWriteMask = 0xf;
            blendAttach1.blendEnable    = VK_FALSE;

            VkPipelineColorBlendAttachmentState blendAttach2{};
            blendAttach2.colorWriteMask = 0xf;
            blendAttach2.blendEnable    = VK_FALSE;

            VkPipelineColorBlendAttachmentState blendAttach3{};
            blendAttach3.colorWriteMask = 0xf;
            blendAttach3.blendEnable    = VK_FALSE;

            VkPipelineColorBlendAttachmentState blendAttach4{};
            blendAttach4.colorWriteMask = 0xf;
            blendAttach4.blendEnable    = VK_FALSE;

            std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {blendAttach1, blendAttach2, blendAttach3, blendAttach4};

            pipelineSpec.colorBlendInfo.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
            pipelineSpec.colorBlendInfo.pAttachments    = blendAttachmentStates.data();

            pipelineSpec.pipelineLayout = pipelineLayout;

            offscreen.pipeline = new VulkanWrapper::VulkanPipeline(m_Device, offscreen.renderPass, pipelineSpec);  // 유니폼 버퍼만 들고 그리는애

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
        }

        /*{  // new outline_pipeline
            auto vertShaderCode = Utils::readFile("shaders/outline.vert.spv");
            auto fragShaderCode = Utils::readFile("shaders/outline.frag.spv");

            VkShaderModule vertShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), vertShaderCode);
            VkShaderModule fragShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName  = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName  = "main";

            pipelineSpec.shaderStages.clear();
            pipelineSpec.shaderStages.push_back(fragShaderStageInfo);
            pipelineSpec.shaderStages.push_back(vertShaderStageInfo);

            pipelineSpec.depthStencilInfo.back.compareOp   = VK_COMPARE_OP_NOT_EQUAL;
            pipelineSpec.depthStencilInfo.back.failOp      = VK_STENCIL_OP_KEEP;
            pipelineSpec.depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
            pipelineSpec.depthStencilInfo.back.passOp      = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.front            = pipelineSpec.depthStencilInfo.back;
            pipelineSpec.depthStencilInfo.depthTestEnable  = VK_FALSE;

            m_pOutlinePipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
        }*/

        {
            // Semaphore for compute & graphics sync
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;  // 이전 graphics pipe에 semaphore 하나 더 생성
            vkCreateSemaphore(m_Device.GetVkLogicalDevice(), &semaphoreCreateInfo, nullptr, &graphics.semaphore);

            // Signal the semaphore
            VkSubmitInfo submitInfo{};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &graphics.semaphore;
            vkQueueSubmit(m_Device.GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_Device.GetVkGraphicsQueue());

            // Create a command buffer for compute operations
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = m_Device.GetVkCommandPool();
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            vkAllocateCommandBuffers(m_Device.GetVkLogicalDevice(), &allocInfo, &offscreen.commandBuffer);

            vkCreateSemaphore(m_Device.GetVkLogicalDevice(), &semaphoreCreateInfo, nullptr, &offscreen.semaphore);

            // compute pipe 구성

            VulkanWrapper::Utils::DescriptorAllocator   descAllocator;
            VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
            descAllocator.init(m_Device.GetVkLogicalDevice());
            descLayoutCache.init(m_Device.GetVkLogicalDevice());

            /*
            VulkanWrapper::Utils::createStorageBuffer(m_Device, &storageBuffer, &storageBufferMemory, sizeof(VulkanWrapper::UniformBufferObject), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = storageBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(VulkanWrapper::UniformBufferObject);

            .bindBuffer(&bufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
            */

            VulkanWrapper::Utils::createUniformBuffer(m_Device, &compute.uniformBuffer, &compute.uniformBufferMemory);
            auto uniformBufferInfo = VulkanWrapper::Utils::createDescBufferInfo(compute.uniformBuffer, 0, sizeof(VulkanWrapper::UniformBufferObject));

            VkDescriptorImageInfo positionImageInfo{};
            positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            positionImageInfo.imageView   = offscreen.position.getImageView();
            positionImageInfo.sampler     = offscreen.position.getSampler();

            VkDescriptorImageInfo normalImageInfo{};
            normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            normalImageInfo.imageView   = offscreen.normal.getImageView();
            normalImageInfo.sampler     = offscreen.normal.getSampler();

            VkDescriptorImageInfo colorImageInfo{};
            colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            colorImageInfo.imageView   = offscreen.color.getImageView();
            colorImageInfo.sampler     = offscreen.color.getSampler();

            VkDescriptorImageInfo uvImageInfo{};
            uvImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            uvImageInfo.imageView   = offscreen.uv.getImageView();
            uvImageInfo.sampler     = offscreen.uv.getSampler();

            VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                .bindBuffer(&uniformBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                .bindImage(&positionImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .bindImage(&normalImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .bindImage(&colorImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .bindImage(&uvImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .build(compute.descriptorSet, compute.descriptorSetLayout, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

            VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &compute.descriptorSetLayout, &compute.pipelineLayout);

            // One pipeline for each effect
            std::vector<std::string> shaderNames;
            shaderNames = {"effectcompute"};
            for (auto& shaderName : shaderNames)
            {
                std::string fileName     = "shaders/" + shaderName + ".comp.spv";
                auto        shaderBinary = Utils::readFile(fileName);

                VkShaderModule compShaderModule =
                    VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), shaderBinary);

                VkPipelineShaderStageCreateInfo compShaderStageInfo{};
                compShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                compShaderStageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
                compShaderStageInfo.module = compShaderModule;
                compShaderStageInfo.pName  = "main";

                pipelineSpec.shaderStages.clear();
                pipelineSpec.shaderStages.push_back(compShaderStageInfo);

                pipelineSpec.pipelineType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

                pipelineSpec.pipelineLayout = compute.pipelineLayout;

                auto computePipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
                compute.pipelines.push_back(std::move(computePipe));

                vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), compShaderModule, nullptr);
            }

            // Separate command pool as queue family for compute may be different than graphics
            VkCommandPoolCreateInfo cmdPoolInfo = {};
            cmdPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolInfo.queueFamilyIndex        = m_Device.GetQueueFamilyIndices().computeFamily.value();
            cmdPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            vkCreateCommandPool(m_Device.GetVkLogicalDevice(), &cmdPoolInfo, nullptr, &compute.commandPool);

            // Create a command buffer for compute operations
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = compute.commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            vkAllocateCommandBuffers(m_Device.GetVkLogicalDevice(), &allocInfo, &compute.commandBuffer);

            // Semaphore for compute & graphics sync
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(m_Device.GetVkLogicalDevice(), &semaphoreCreateInfo, nullptr, &compute.semaphore);

            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(m_Device.GetVkLogicalDevice(), &fenceCreateInfo, nullptr, &compute.fence);

            compute.queue = m_Device.GetVkComputeQueue();
        }

        // --

        startTime = std::chrono::high_resolution_clock::now();

        camera.setViewTarget(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0));
        float aspect = m_Swapchain.GetVkExtent2D().width / (float)m_Swapchain.GetVkExtent2D().height;
        camera.setPerspectiveProjection(glm::radians(45.f), aspect, 0.1f, 100.f);
    }

    Application::~Application()
    {
        glfwTerminate();
    }

    void Application::run()
    {
        mainLoop();
    }

    void Application::mainLoop()
    {
        while (!glfwWindowShouldClose(m_Window.GetGLFWWindow()))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(m_Device.GetVkLogicalDevice());
    }

    void Application::drawFrame()
    {
        auto currentFrame  = m_Renderer.currentFrame;
        auto currentBuffer = m_Renderer.GetCurrentBuffer();

        vkResetCommandBuffer(currentBuffer, 0);            // delete
        vkResetCommandBuffer(offscreen.commandBuffer, 0);  // delete
        vkResetCommandBuffer(compute.commandBuffer, 0);    // delete

        // calc delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime   = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
        startTime        = currentTime;


        //vkQueueWaitIdle(compute.queue);
        //vkQueueWaitIdle(m_Device.GetVkGraphicsQueue());

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(compute.commandBuffer, &cmdBufferBeginInfo);

        vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[compute.pipelineIndex]->GetVulkanPipeline());


                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = gameObjects[0].model->getMeshes()[0].getMaterial().getUniformBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range  = sizeof(VulkanWrapper::UniformBufferObject);

                VkDescriptorImageInfo positionMapInfo{};
                positionMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                positionMapInfo.imageView   = offscreen.position.getImageView();
                positionMapInfo.sampler     = offscreen.frameSampler;

                VkDescriptorImageInfo normalMapInfo{};
                normalMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                normalMapInfo.imageView   = offscreen.normal.getImageView();
                normalMapInfo.sampler     = offscreen.frameSampler;

                VkDescriptorImageInfo colorMapInfo{};
                colorMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                colorMapInfo.imageView   = offscreen.color.getImageView();
                colorMapInfo.sampler     = offscreen.frameSampler;

                VkDescriptorImageInfo uvMapInfo{};
                uvMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                uvMapInfo.imageView   = offscreen.uv.getImageView();
                uvMapInfo.sampler     = offscreen.frameSampler;

                std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};

                // Scene matrices
                writeDescriptorSets[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[0].dstSet          = 0;
                writeDescriptorSets[0].dstBinding      = 0;
                writeDescriptorSets[0].descriptorCount = 1;
                writeDescriptorSets[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSets[0].pBufferInfo     = &bufferInfo;

                writeDescriptorSets[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[1].dstSet          = 0;
                writeDescriptorSets[1].dstBinding      = 1;
                writeDescriptorSets[1].descriptorCount = 1;
                writeDescriptorSets[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[1].pImageInfo      = &positionMapInfo;

                writeDescriptorSets[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[2].dstSet          = 0;
                writeDescriptorSets[2].dstBinding      = 2;
                writeDescriptorSets[2].descriptorCount = 1;
                writeDescriptorSets[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[2].pImageInfo      = &normalMapInfo;

                writeDescriptorSets[3].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[3].dstSet          = 0;
                writeDescriptorSets[3].dstBinding      = 3;
                writeDescriptorSets[3].descriptorCount = 1;
                writeDescriptorSets[3].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[3].pImageInfo      = &colorMapInfo;

                writeDescriptorSets[4].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[4].dstSet          = 0;
                writeDescriptorSets[4].dstBinding      = 4;
                writeDescriptorSets[4].descriptorCount = 1;
                writeDescriptorSets[4].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[4].pImageInfo      = &uvMapInfo;

                m_Device.vkCmdPushDescriptorSetKHR(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 5, writeDescriptorSets.data());

                vkCmdDispatch(compute.commandBuffer, offscreen.width / 16, offscreen.height / 16, 1);
                /*
                // color는 끝나야 실행
                VkImageMemoryBarrier imageMemoryBarrier = {};
                imageMemoryBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                // We won't be changing the layout of the image
                imageMemoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_GENERAL;
                imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
                imageMemoryBarrier.image               = offscreen.color.getImage();  // ?
                imageMemoryBarrier.subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                imageMemoryBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
                imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                // 합쳐서 그리는애
                // 파이프라인이 dst(fragment)에 도착했을때 compute 단계가 done이 되면 실행
                // queue의 전후 기록된 명령간 dependency 생성
                vkCmdPipelineBarrier(
                    currentBuffer,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &imageMemoryBarrier);
                */


        vkEndCommandBuffer(compute.commandBuffer);

        VkCommandBufferBeginInfo cmdBufInfo{};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // Clear values for all attachments written in the fragment shader
        std::array<VkClearValue, 5> clearValues;
        clearValues[0].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[1].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[2].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[3].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[4].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass               = offscreen.renderPass;
        renderPassBeginInfo.framebuffer              = offscreen.frameBuffer;
        renderPassBeginInfo.renderArea.extent.width  = offscreen.width;
        renderPassBeginInfo.renderArea.extent.height = offscreen.height;
        renderPassBeginInfo.clearValueCount          = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues             = clearValues.data();

        vkBeginCommandBuffer(offscreen.commandBuffer, &cmdBufInfo);

        vkCmdBeginRenderPass(offscreen.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.width    = (float)offscreen.width;
        viewport.height   = (float)offscreen.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(offscreen.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{offscreen.width, offscreen.height, 0, 0};
        vkCmdSetScissor(offscreen.commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(offscreen.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline->GetVulkanPipeline());

        // process keyboard event
        for (auto& go : gameObjects)
        {
            keyboardListener.moveInPlaneXZ(m_Window.GetGLFWWindow(), deltaTime, go);
        }

        renderGameObjects(offscreen.commandBuffer);

        vkCmdEndRenderPass(offscreen.commandBuffer);

        vkEndCommandBuffer(offscreen.commandBuffer);

        uint32_t imageIndex;
        VkResult result = m_Swapchain.AcquireNextImage(&imageIndex, currentFrame);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }


        m_Renderer.BeginDraw();

        m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);

        VkDescriptorImageInfo colorMapInfo2{};
        colorMapInfo2.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        colorMapInfo2.imageView   = offscreen.color.getImageView();
        colorMapInfo2.sampler     = offscreen.color.getSampler();

        std::array<VkWriteDescriptorSet, 1> writeDescriptorSets2{};

        writeDescriptorSets2[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets2[0].dstSet          = 0;
        writeDescriptorSets2[0].dstBinding      = 0;
        writeDescriptorSets2[0].descriptorCount = 1;
        writeDescriptorSets2[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets2[0].pImageInfo      = &colorMapInfo2;

        m_Device.vkCmdPushDescriptorSetKHR(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, writeDescriptorSets2.data());

        m_pPipeline->bind(currentBuffer);

        vkCmdDraw(currentBuffer, 3, 1, 0, 0);

        m_Swapchain.EndRenderPass(currentBuffer);
        VulkanWrapper::endCommandBuffer(currentBuffer);

        // to all command buffers

        vkWaitForFences(m_Device.GetVkLogicalDevice(), 1, &compute.fence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_Device.GetVkLogicalDevice(), 1, &compute.fence);

        VkPipelineStageFlags offscreenwaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // Submit offscreen commands
        VkSubmitInfo offscreenSubmitInfo{};
        offscreenSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        offscreenSubmitInfo.commandBufferCount   = 1;
        offscreenSubmitInfo.pCommandBuffers      = &offscreen.commandBuffer;
        offscreenSubmitInfo.waitSemaphoreCount   = 1;
        offscreenSubmitInfo.pWaitSemaphores      = &graphics.semaphore;  
        offscreenSubmitInfo.pWaitDstStageMask    = &offscreenwaitStageMask;
        offscreenSubmitInfo.signalSemaphoreCount = 1;
        offscreenSubmitInfo.pSignalSemaphores    = &offscreen.semaphore; 
        VK_CHECK_RESULT(vkQueueSubmit(m_Device.GetVkGraphicsQueue(), 1, &offscreenSubmitInfo, VK_NULL_HANDLE));

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        // Submit compute commands
        VkSubmitInfo computeSubmitInfo{};
        computeSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        computeSubmitInfo.commandBufferCount   = 1;
        computeSubmitInfo.pCommandBuffers      = &compute.commandBuffer;
        computeSubmitInfo.waitSemaphoreCount   = 1;
        computeSubmitInfo.pWaitSemaphores      = &offscreen.semaphore;  // 기다리는 세마포어 -> 이전 렌더링 finish
        computeSubmitInfo.pWaitDstStageMask    = &waitStageMask;
        computeSubmitInfo.signalSemaphoreCount = 1;
        computeSubmitInfo.pSignalSemaphores    = &compute.semaphore;  // rendering finish singal 받는 semaphore pointer
        VK_CHECK_RESULT(vkQueueSubmit(compute.queue, 1, &computeSubmitInfo, compute.fence));

        VkPipelineStageFlags graphicsWaitStageMasks[]   = {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore          graphicsWaitSemaphores[]   = {compute.semaphore, m_Swapchain.imageAvailableSemaphores[currentFrame]};   // 컴퓨트가 present가 끝나면
        VkSemaphore          graphicsSignalSemaphores[] = {graphics.semaphore, m_Swapchain.renderFinishedSemaphores[currentFrame]};  // 그래픽 세마포어에 렌더링 finish signal 보냄

        // Submit graphics commands
        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &currentBuffer;
        submitInfo.waitSemaphoreCount   = 2;
        submitInfo.pWaitSemaphores      = graphicsWaitSemaphores;
        submitInfo.pWaitDstStageMask    = graphicsWaitStageMasks;
        submitInfo.signalSemaphoreCount = 2;
        submitInfo.pSignalSemaphores    = graphicsSignalSemaphores;
        vkQueueSubmit(m_Device.GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

        VkSemaphore signalSemaphores[]  = {m_Swapchain.renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_Swapchain.GetVkSwapChain()};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;
        presentInfo.pImageIndices   = &imageIndex;
        presentInfo.pResults        = nullptr;

        result = vkQueuePresentKHR(m_Device.GetVkPresentQueue(), &presentInfo);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
        m_Renderer.EndDraw();

        vkQueueWaitIdle(m_Device.GetVkPresentQueue());
    }

    void Application::loadGameObjects()
    {
        //        auto model = std::make_shared<Model>(m_Device, "models/smooth_vase.obj");
        //        auto testGO = GameObject::createGameObject();
        //        testGO.model = model;
        //        testGO.transform.position = glm::vec3(0.5f, -0.5f, 0.f);
        //        gameObjects.push_back(std::move(testGO));

        auto model2 = std::make_shared<Model>(m_Device, "models/untitled.obj");
        //auto model2  = std::make_shared<Model>(m_Device, "models/sphere.obj");
        auto testGO2            = GameObject::createGameObject();
        testGO2.transform.scale = glm::vec3(0.1f);
        //testGO2.transform.scale    = glm::vec3(0.01f);
        testGO2.transform.rotation = glm::vec3(0.0, 3.14, 0.0);
        testGO2.model              = model2;
        testGO2.transform.position = glm::vec3(0.f, -0.5f, 0.f);
        gameObjects.push_back(std::move(testGO2));

        auto model3                = std::make_shared<Model>(m_Device, "models/sphere.obj");  // light obj
        auto testGO3               = GameObject::createGameObject();
        testGO3.model              = model3;
        testGO3.transform.scale    = glm::vec3(0.005f);
        testGO3.transform.position = glm::vec3(-0.0, 2.0, -2.0);
        gameObjects.push_back(std::move(testGO3));
    }

    void Application::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        for (auto& go : gameObjects)
        {
            Myu::VulkanWrapper::UniformBufferObject ubo{};
            ubo.model = go.transform.toMat4();
            ubo.view  = camera.getView();
            ubo.proj  = camera.getProjection();

            /*
            if (go.transform.scale == glm::vec3(0.005f))  // light
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 2.0);
            else
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 1.0);

            ubo.gLight[0].color = glm::vec4(glm::vec3(1.0, 1.0, 1.0), 0.3);
            */
            // FIMXE: HACK

            //m_pOutlinePipe->bind(commandBuffer);
            //go.model->bind(commandBuffer, pipelineLayout, ubo);

            //m_pPipeline->bind(commandBuffer);
            go.model->bind(commandBuffer, pipelineLayout, ubo);
        }
    }
}  // namespace Myu
