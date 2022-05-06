#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"

#include <chrono>
#include "Debug.hpp"
#include "../VulkanWrapper/Utils.hpp"
#include "../VulkanWrapper/VulkanTexture.hpp"

VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout      pipelineLayout;
VkPipeline            graphicsPipeline;

VkImage        textureImage;
VkDeviceMemory textureImageMemory;
VkImageView    textureImageView;
VkSampler      textureSampler;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
Myu::KeyboardListener keyboardListener {};
Myu::Camera camera = Myu::Camera();

const std::string TEXTURE_PATH = "textures/viking_room.png";

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
        
        // creatae descriptor set layout
        auto uniformBinding = VulkanWrapper::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
        auto samplerBinding = VulkanWrapper::createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
        std::vector<VkDescriptorSetLayoutBinding> bindings { uniformBinding, samplerBinding };
        VulkanWrapper::createDescriptorSetLayout(m_Device.GetVkLogicalDevice(), bindings, &descriptorSetLayout);
        
        // create descriptor set
        for (auto& go : gameObjects)
        {
            auto texture = VulkanWrapper::VulkanTexture();
            texture.loadFromFile(&m_Device, "textures/viking_room.png");
            VulkanWrapper::Utils::createDescriptorSet(m_Device.GetVkLogicalDevice(), m_Device.GetVkDescriptorPool(), descriptorSetLayout, go.model->getDescriptorSet(), go.model->getUniformBuffer(), texture.getImageView(), texture.getSampler());
        }
        
        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));

        VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};
        pipelineSpec.vertFilepath   = "shaders/vert.spv";
        pipelineSpec.fragFilepath   = "shaders/frag.spv";
        pipelineSpec.pipelineLayout = pipelineLayout;
        
        // viewport info setup
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_Swapchain.GetVkExtent2D().width;
        viewport.height = (float) m_Swapchain.GetVkExtent2D().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        pipelineSpec.viewportInfo.pViewports = &viewport;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_Swapchain.GetVkExtent2D();
        pipelineSpec.viewportInfo.pScissors = &scissor;
        
        m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
        
        // create texture relevent resources
        VulkanWrapper::createTextureSampler(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(), textureSampler);
        
        startTime = std::chrono::high_resolution_clock::now();
        
        camera.setViewTarget(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0));
        float aspect = m_Swapchain.GetVkExtent2D().width / (float)m_Swapchain.GetVkExtent2D().height;
        camera.setPerspectiveProjection(glm::radians(45.f), aspect, 0.1f, 10.f);
    }

    Application::~Application()
    {
        vkDestroySampler(m_Device.GetVkLogicalDevice(), textureSampler, nullptr);
        vkDestroyImageView(m_Device.GetVkLogicalDevice(), textureImageView, nullptr);

        vkDestroyImage(m_Device.GetVkLogicalDevice(), textureImage, nullptr);
        vkFreeMemory(m_Device.GetVkLogicalDevice(), textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(m_Device.GetVkLogicalDevice(), descriptorSetLayout, nullptr);

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
        auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
        startTime = currentTime;

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
        
        auto model2 = std::make_shared<Model>(m_Device, "models/viking_room.obj");
        auto testGO2 = GameObject::createGameObject();
        testGO2.model = model2;
        testGO2.transform.position = glm::vec3(0.f, 0.f, 0.f);
        gameObjects.push_back(std::move(testGO2));
    }

    void Application::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        m_pPipeline->bind(commandBuffer);
        
        for (auto& go : gameObjects)
        {
            VulkanWrapper::updateUniformBuffer(m_Device.GetVkLogicalDevice(), m_Swapchain.GetVkExtent2D(), go.model->getUniformMemory(), go.transform.toMat4(), camera.getView(), camera.getProjection());
            VulkanWrapper::bindDescriptorSet(commandBuffer, pipelineLayout, go.model->getDescriptorSet());
            
            go.model->bind(commandBuffer);
            
            go.model->draw(commandBuffer);
        }
    }
}
