#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"

#include <chrono>
#include "../VulkanWrapper/VulkanTexture.hpp"
#include "../VulkanWrapper/VulkanInitializer.hpp"

#include <glm/gtc/type_ptr.hpp>

VkPipelineLayout      pipelineLayout;

VkPipelineLayout      computePipeLayout;
VkDescriptorSetLayout computeDescLayout;
VkDescriptorSet       computeDescSet;
VkBuffer              storageBuffer;
VkDeviceMemory        storageBufferMemory;
VkBuffer              storageBuffer2;
VkDeviceMemory        storageBufferMemory2;
VkBuffer              storageBuffer3;
VkDeviceMemory        storageBufferMemory3;

VkPipeline graphicsPipeline;

VkImage                                                     textureImage;
VkDeviceMemory                                              textureImageMemory;
VkImageView                                                 textureImageView;
VkSampler                                                   textureSampler;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
Myu::KeyboardListener                                       keyboardListener{};
Myu::Camera                                                 camera = Myu::Camera();

std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
VkDescriptorPool             m_ImGuiDescriptorPool;
VkCommandPool                m_ImGuiCommandPool;
std::vector<VkFramebuffer>   m_ImGuiFramebuffers;
VkRenderPass                 m_ImGuiRenderPass;

std::vector<VkDescriptorSet> descriptorSets(Myu::VulkanWrapper::MAX_FRAMES_IN_FLIGHT);

namespace Myu
{
    Application::Application()
    {
        loadGameObjects();

        //FIXME: HACK
        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &gameObjects[0].model->getMeshes()[0].getMaterial().getDescriptorLayout(), &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));

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

        pipelineSpec.bindingDescriptions   = Myu::VulkanWrapper::Vertex::getBindingDescription();
        pipelineSpec.attributeDescriptions = Myu::VulkanWrapper::Vertex::getAttributeDescriptions();

        {
            auto vertShaderCode = Utils::readFile("shaders/shader.vert.spv");
            auto fragShaderCode = Utils::readFile("shaders/shader.frag.spv");

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

            pipelineSpec.shaderStages.push_back(fragShaderStageInfo);
            pipelineSpec.shaderStages.push_back(vertShaderStageInfo);
            pipelineSpec.rasterizationInfo.cullMode         = VK_CULL_MODE_NONE;
            pipelineSpec.depthStencilInfo.stencilTestEnable = VK_TRUE;
            pipelineSpec.depthStencilInfo.back.compareOp    = VK_COMPARE_OP_ALWAYS;
            pipelineSpec.depthStencilInfo.back.failOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.depthFailOp  = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.passOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.compareMask  = 0xff;
            pipelineSpec.depthStencilInfo.back.writeMask    = 0xff;
            pipelineSpec.depthStencilInfo.back.reference    = 1;
            pipelineSpec.depthStencilInfo.front             = pipelineSpec.depthStencilInfo.back;
            pipelineSpec.pipelineLayout = pipelineLayout;

            m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
        }
        {  // new outline_pipeline

            /*
            pipelineSpec.vertFilepath                       = "shaders/texture.vert.spv";
            pipelineSpec.fragFilepath                       = "shaders/texture.frag.spv";

            pipelineSpec.rasterizationInfo.cullMode         = VK_CULL_MODE_NONE;
            pipelineSpec.depthStencilInfo.stencilTestEnable = VK_TRUE;
            pipelineSpec.depthStencilInfo.back.compareOp    = VK_COMPARE_OP_ALWAYS;
            pipelineSpec.depthStencilInfo.back.failOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.depthFailOp  = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.passOp       = VK_STENCIL_OP_REPLACE;
            pipelineSpec.depthStencilInfo.back.compareMask  = 0xff;
            pipelineSpec.depthStencilInfo.back.writeMask    = 0xff;
            pipelineSpec.depthStencilInfo.back.reference    = 1;
            pipelineSpec.depthStencilInfo.front             = pipelineSpec.depthStencilInfo.back;

            pipeline_stencil = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
            */

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
        }
        {
            VulkanWrapper::Utils::DescriptorAllocator   descAllocator;
            VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
            descAllocator.init(m_Device.GetVkLogicalDevice());
            descLayoutCache.init(m_Device.GetVkLogicalDevice());

            VulkanWrapper::Utils::createStorageBuffer(m_Device, sizeof(VulkanWrapper::UniformBufferObject), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer, &storageBufferMemory);
            VulkanWrapper::Utils::createStorageBuffer(m_Device, sizeof(VulkanWrapper::VertexObject), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer2, &storageBufferMemory2);
            VulkanWrapper::Utils::createStorageBuffer(m_Device, sizeof(glm::mat4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer3, &storageBufferMemory3);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = storageBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(VulkanWrapper::UniformBufferObject);

            VkDescriptorBufferInfo bufferInfo2{};
            bufferInfo2.buffer = storageBuffer2;
            bufferInfo2.offset = 0;
            bufferInfo2.range  = sizeof(VulkanWrapper::VertexObject);

            VkDescriptorBufferInfo bufferInfo3{};
            bufferInfo3.buffer = storageBuffer3;
            bufferInfo3.offset = 0;
            bufferInfo3.range  = sizeof(glm::mat4);
           
            VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
            .bindBuffer(&bufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
            .bindBuffer(&bufferInfo2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
            .bindBuffer(&bufferInfo3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT)
            .build(computeDescSet, computeDescLayout);

            VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &computeDescLayout, &computePipeLayout);

            auto compShaderCode = Utils::readFile("shaders/postprocessing.comp.spv");

            VkShaderModule compShaderModule =
                VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), compShaderCode);

            pipelineSpec.bindingDescriptions.clear();
            pipelineSpec.attributeDescriptions.clear();

            VkPipelineShaderStageCreateInfo compShaderStageInfo{};
            compShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            compShaderStageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
            compShaderStageInfo.module = compShaderModule;
            compShaderStageInfo.pName  = "main";

            pipelineSpec.shaderStages.clear();
            pipelineSpec.shaderStages.push_back(compShaderStageInfo);

            pipelineSpec.pipelineType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

            m_pPostPipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), compShaderModule, nullptr);

            //VkCommandBuffer commandbuf = VulkanWrapper::beginSingleTimeCommands(m_Device.GetVkLogicalDevice(), m_Device.GetVkCommandPool());
            //vkCmdBindPipeline(commandbuf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pPostPipe->GetVulkanPipeline());
            //vkCmdBindDescriptorSets(commandbuf, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeLayout, 0, 1, &computeDescSet, 0, 0);
            //vkCmdDispatch(commandbuf, 1, 0, 1);
            //VulkanWrapper::endSingleTimeCommands(m_Device.GetVkLogicalDevice(), commandbuf, m_Device.GetVkGraphicsQueue(), m_Device.GetVkCommandPool());

        }
        // create texture relevent resources
        VulkanWrapper::createTextureSampler(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(), textureSampler);

        startTime = std::chrono::high_resolution_clock::now();

        camera.setViewTarget(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0));
        float aspect = m_Swapchain.GetVkExtent2D().width / (float)m_Swapchain.GetVkExtent2D().height;
        camera.setPerspectiveProjection(glm::radians(45.f), aspect, 0.1f, 100.f);
    }

    Application::~Application()
    {
        vkDestroySampler(m_Device.GetVkLogicalDevice(), textureSampler, nullptr);
        vkDestroyImageView(m_Device.GetVkLogicalDevice(), textureImageView, nullptr);

        vkDestroyImage(m_Device.GetVkLogicalDevice(), textureImage, nullptr);
        vkFreeMemory(m_Device.GetVkLogicalDevice(), textureImageMemory, nullptr);

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

        // calc delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime   = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
        startTime        = currentTime;

        uint32_t imageIndex;
        VkResult result = m_Swapchain.AcquireNextImage(&imageIndex, currentFrame);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetCommandBuffer(currentBuffer, 0);

        m_Renderer.BeginDraw();

        m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);

        // process keyboard event
        for (auto& go : gameObjects)
        {
            keyboardListener.moveInPlaneXZ(m_Window.GetGLFWWindow(), deltaTime, go);
        }

        renderGameObjects(currentBuffer);

        m_Swapchain.EndRenderPass(currentBuffer);
        VulkanWrapper::endCommandBuffer(currentBuffer);

        std::vector bufs{currentBuffer};
        result = m_Swapchain.PresentQueue(bufs, imageIndex, currentFrame);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_Renderer.EndDraw();
        /*
        void*    bufferData;
        result = vkMapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory3, 0, VK_WHOLE_SIZE, 0, &bufferData);
        std::cout << result << '\n';
        //glm::vec4 recivedData = glm::make_vec4(bufferData);
        std::cout << static_cast<char*>(bufferData) << '\n';
        vkUnmapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory3);
        */
    }

    void Application::loadGameObjects()
    {
        //        auto model = std::make_shared<Model>(m_Device, "models/smooth_vase.obj");
        //        auto testGO = GameObject::createGameObject();
        //        testGO.model = model;
        //        testGO.transform.position = glm::vec3(0.5f, -0.5f, 0.f);
        //        gameObjects.push_back(std::move(testGO));

        //auto model2                = std::make_shared<Model>(m_Device, "models/untitled.obj");
        auto model2  = std::make_shared<Model>(m_Device, "models/sphere.obj");
        auto testGO2               = GameObject::createGameObject();
        //testGO2.transform.scale    = glm::vec3(0.1f);
        testGO2.transform.scale    = glm::vec3(0.01f);
        testGO2.transform.rotation = glm::vec3(0.0, 3.14, 0.0);
        testGO2.model              = model2;
        testGO2.transform.position = glm::vec3(0.f, -0.5f, 0.f);
        gameObjects.push_back(std::move(testGO2));

        auto model3                = std::make_shared<Model>(m_Device, "models/sphere.obj"); // light obj
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
            ubo.model              = go.transform.toMat4();
            ubo.view               = camera.getView();
            ubo.proj               = camera.getProjection();
            
            if (go.transform.scale == glm::vec3(0.005f)) // light
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 2.0);
            else
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 1.0);

            ubo.gLight[0].color = glm::vec4(glm::vec3(1.0, 1.0, 1.0), 0.3);
            // FIMXE: HACK

            //m_pOutlinePipe->bind(commandBuffer);
            //go.model->bind(commandBuffer, pipelineLayout, ubo);

            m_pPipeline->bind(commandBuffer);
            go.model->bind(commandBuffer, pipelineLayout, ubo);


            /*
            VkBufferMemoryBarrier bufferBarrier = vks::initializers::bufferMemoryBarrier();
            bufferBarrier.buffer                = indirectCommandsBuffer.buffer;
            bufferBarrier.size                  = indirectCommandsBuffer.descriptor.range;
            bufferBarrier.srcAccessMask         = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            bufferBarrier.dstAccessMask         = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.srcQueueFamilyIndex   = vulkanDevice->queueFamilyIndices.graphics;
            bufferBarrier.dstQueueFamilyIndex   = vulkanDevice->queueFamilyIndices.compute;

            vkCmdPipelineBarrier(
                compute.commandBuffer,
                VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_FLAGS_NONE,
                0,
                nullptr,
                1,
                &bufferBarrier,
                0,
                nullptr);
            

            void* data;
            vkMapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory);

            Myu::VulkanWrapper::VertexObject v{};
            vkMapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory2, 0, sizeof(v), 0, &data);
            memcpy(data, &v, sizeof(v));
            vkUnmapMemory(m_Device.GetVkLogicalDevice(), storageBufferMemory2);
            */
        }
    }
}  // namespace Myu
