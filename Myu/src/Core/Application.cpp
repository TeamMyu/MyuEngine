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

    Myu::VulkanWrapper::VulkanTexture inputTexture;
    Myu::VulkanWrapper::VulkanTexture outputTexture;
} compute;

struct
{
    VkDescriptorSetLayout descriptorSetLayout;       // Image display shader binding layout
    VkDescriptorSet       descriptorSetPreCompute;   // Image display shader bindings before compute shader image manipulation
    VkDescriptorSet       descriptorSetPostCompute;  // Image display shader bindings after compute shader image manipulation
    VkPipeline            pipeline;                  // Image display pipeline
    VkPipelineLayout      pipelineLayout;            // Layout of the graphics pipeline
    VkSemaphore           semaphore;                 // Execution dependency between compute & graphic submission
} graphics;

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

        compute.inputTexture  = gameObjects[0].model->getMeshes()[0].getMaterial().inputTexture;
        compute.outputTexture = gameObjects[0].model->getMeshes()[0].getMaterial().outputTexture;

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
            pipelineSpec.pipelineLayout                     = pipelineLayout;

            m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

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

            VkDescriptorImageInfo inputImageInfo{};
            inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            inputImageInfo.imageView   = compute.inputTexture.getImageView();
            inputImageInfo.sampler     = compute.inputTexture.getSampler();

            VkDescriptorImageInfo outputImageInfo{};
            outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            outputImageInfo.imageView   = compute.outputTexture.getImageView();
            outputImageInfo.sampler     = compute.outputTexture.getSampler();

            VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                .bindImage(&inputImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .bindImage(&outputImageInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .build(compute.descriptorSet, compute.descriptorSetLayout, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

            VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &compute.descriptorSetLayout, &compute.pipelineLayout);

            // One pipeline for each effect
            std::vector<std::string> shaderNames;
            shaderNames = {"postprocessing"};
            for (auto& shaderName : shaderNames)
            {
                std::string fileName     = "shaders/" + shaderName + ".comp.spv";
                auto        shaderBinary = Utils::readFile(fileName);

                VkShaderModule compShaderModule =
                    VulkanWrapper::Utils::createShaderModule(m_Device.GetVkLogicalDevice(), shaderBinary);

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
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = compute.commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            vkAllocateCommandBuffers(m_Device.GetVkLogicalDevice(), &allocInfo, &compute.commandBuffer);

            // Semaphore for compute & graphics sync
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(m_Device.GetVkLogicalDevice(), &semaphoreCreateInfo, nullptr, &compute.semaphore);

            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(m_Device.GetVkLogicalDevice(), &fenceCreateInfo, nullptr, &compute.fence);

            compute.queue = m_Device.GetVkComputeQueue();
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

        vkResetCommandBuffer(currentBuffer, 0);  // delete
        vkResetCommandBuffer(compute.commandBuffer, 0);  // delete
        // to all command buffers
        m_Renderer.BeginDraw();

        vkQueueWaitIdle(compute.queue);

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(compute.commandBuffer, &cmdBufferBeginInfo);

        vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[compute.pipelineIndex]->GetVulkanPipeline());

        for (auto& go : gameObjects)
        {
  

            for (auto mesh : go.model->getMeshes())
            {
                VkDescriptorImageInfo inputImageInfo{};
                inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                inputImageInfo.imageView   = mesh.getMaterial().inputTexture.getImageView();
                inputImageInfo.sampler     = mesh.getMaterial().inputTexture.getSampler();

                VkDescriptorImageInfo outputImageInfo{};
                outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                outputImageInfo.imageView   = mesh.getMaterial().outputTexture.getImageView();
                outputImageInfo.sampler     = mesh.getMaterial().outputTexture.getSampler();

                std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};

                writeDescriptorSets[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[0].dstSet          = 0;
                writeDescriptorSets[0].dstBinding      = 0;
                writeDescriptorSets[0].descriptorCount = 1;
                writeDescriptorSets[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[0].pImageInfo      = &inputImageInfo;

                writeDescriptorSets[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[1].dstSet          = 0;
                writeDescriptorSets[1].dstBinding      = 1;
                writeDescriptorSets[1].descriptorCount = 1;
                writeDescriptorSets[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptorSets[1].pImageInfo      = &outputImageInfo;

                m_Device.vkCmdPushDescriptorSetKHR(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 2, writeDescriptorSets.data());

                //vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);
                vkCmdDispatch(compute.commandBuffer, compute.outputTexture.width / 16, compute.outputTexture.height / 16, 1);

                VkImageMemoryBarrier imageMemoryBarrier = {};
                imageMemoryBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                // We won't be changing the layout of the image
                imageMemoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_GENERAL;
                imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
                imageMemoryBarrier.image               = mesh.getMaterial().outputTexture.getImage();
                imageMemoryBarrier.subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                imageMemoryBarrier.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
                imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

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
            }
            
        }
        vkEndCommandBuffer(compute.commandBuffer);

        m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);

        // process keyboard event
        for (auto& go : gameObjects)
        {
            keyboardListener.moveInPlaneXZ(m_Window.GetGLFWWindow(), deltaTime, go);
        }

        renderGameObjects(currentBuffer);

        m_Swapchain.EndRenderPass(currentBuffer);
        VulkanWrapper::endCommandBuffer(currentBuffer);

        // to all command buffers

        vkWaitForFences(m_Device.GetVkLogicalDevice(), 1, &compute.fence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_Device.GetVkLogicalDevice(), 1, &compute.fence);

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

        // Submit compute commands
        VkSubmitInfo computeSubmitInfo{};
        computeSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        computeSubmitInfo.commandBufferCount   = 1;
        computeSubmitInfo.pCommandBuffers      = &compute.commandBuffer;
        computeSubmitInfo.waitSemaphoreCount   = 1;
        computeSubmitInfo.pWaitSemaphores      = &graphics.semaphore;  // 기다리는 세마포어 -> 이전 렌더링 finish
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

            if (go.transform.scale == glm::vec3(0.005f))  // light
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 2.0);
            else
                ubo.gLight[0].position = glm::vec4(glm::vec3(-0.0, 2.0, -2.0), 1.0);

            ubo.gLight[0].color = glm::vec4(glm::vec3(1.0, 1.0, 1.0), 0.3);
            // FIMXE: HACK

            //m_pOutlinePipe->bind(commandBuffer);
            //go.model->bind(commandBuffer, pipelineLayout, ubo);

            m_pPipeline->bind(commandBuffer);
            go.model->bind(commandBuffer, pipelineLayout, ubo);
        }
    }
}  // namespace Myu
