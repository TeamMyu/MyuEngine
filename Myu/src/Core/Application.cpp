#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"

#include <chrono>
#include "../VulkanWrapper/VulkanTexture.hpp"
#include "../VulkanWrapper/VulkanInitializer.hpp"

VkPipelineLayout      pipelineLayout;
VkPipelineLayout      computePipeLayout;
VkDescriptorSetLayout computeDescLayout;
VkDescriptorSet       computeDescSet;

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

            pipelineSpec.pipelineLayout = pipelineLayout;

            m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
        }
        /*{
            auto vertShaderCode = Utils::readFile("shaders/specular.vert.spv");
            auto fragShaderCode = Utils::readFile("shaders/specular.frag.spv");

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

            m_pSpecularPipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), vertShaderModule, nullptr);
            vkDestroyShaderModule(m_Device.GetVkLogicalDevice(), fragShaderModule, nullptr);
        }*/
        {
            
            auto storageBinding  = VulkanWrapper::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            auto storageBinding2 = VulkanWrapper::createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);

            std::vector<VkDescriptorSetLayoutBinding> bindings{storageBinding2, storageBinding};

            VulkanWrapper::createDescriptorSetLayout(m_Device.GetVkLogicalDevice(), bindings, &computeDescLayout);

            VulkanWrapper::Utils::DescriptorAllocator   descAllocator;
            VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
            descAllocator.init(m_Device.GetVkLogicalDevice());
            descLayoutCache.init(m_Device.GetVkLogicalDevice());

            VkBuffer       storageBuffer;
            VkDeviceMemory storageBufferMemory;
            VkBuffer       storageBuffer2;
            VkDeviceMemory storageBufferMemory2;

            VulkanWrapper::Utils::createStorageBuffer(m_Device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer, &storageBufferMemory);
            VulkanWrapper::Utils::createStorageBuffer(m_Device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer2, &storageBufferMemory2);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = storageBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = 16;

            VkDescriptorBufferInfo bufferInfo2{};
            bufferInfo2.buffer = storageBuffer2;
            bufferInfo2.offset = 0;
            bufferInfo2.range  = 16;

            auto builder = VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                               .bindBuffer(&bufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                               .bindBuffer(&bufferInfo2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                               .build(computeDescSet, computeDescLayout);

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_Device.GetVkDescriptorPool();
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts        = &computeDescLayout;

            if (vkAllocateDescriptorSets(m_Device.GetVkLogicalDevice(), &allocInfo, &computeDescSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = computeDescSet;
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &bufferInfo;
            vkUpdateDescriptorSets(m_Device.GetVkLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

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
    }

    void Application::loadGameObjects()
    {
        //        auto model = std::make_shared<Model>(m_Device, "models/smooth_vase.obj");
        //        auto testGO = GameObject::createGameObject();
        //        testGO.model = model;
        //        testGO.transform.position = glm::vec3(0.5f, -0.5f, 0.f);
        //        gameObjects.push_back(std::move(testGO));

        auto model2                = std::make_shared<Model>(m_Device, "models/paimon.obj");
        auto testGO2               = GameObject::createGameObject();
        testGO2.model              = model2;
        testGO2.transform.position = glm::vec3(0.f, 0.f, 0.f);
        gameObjects.push_back(std::move(testGO2));
    }

    void Application::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        m_pPipeline->bind(commandBuffer);

        for (auto& go : gameObjects)
        {
            Myu::VulkanWrapper::UniformBufferObject ubo{};
            go.transform.scale     = glm::vec3(0.1f);
            ubo.model              = go.transform.toMat4();
            ubo.view               = camera.getView();
            ubo.proj               = camera.getProjection();
            ubo.pLight[0].position = glm::vec4(glm::vec3(-0.0, -1.0, -0.5), 1.0);
            ubo.pLight[0].color    = glm::vec4(glm::vec3(0.25, 0.75, 1.0), 1.0);
            // FIMXE: HACK
            go.model->bind(commandBuffer, pipelineLayout, ubo);

            //go.model->draw(commandBuffer);
        }
    }
}  // namespace Myu
