
#include "Application.hpp"

VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

VkImage textureImage;
VkDeviceMemory textureImageMemory;
VkImageView textureImageView;
VkSampler textureSampler;

std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;


std::vector<VkDescriptorSet> descriptorSets;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

VulkanModel *m_Model;

Application::Application() 
{
    createDescriptorSetLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout);
    createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(PushConstantObject));

    VulkanPipelineSpecification pipelineSpec;
    pipelineSpec.vertFilepath = "shaders/vert.spv";
    pipelineSpec.fragFilepath = "shaders/frag.spv";
    pipelineSpec.pipelineLayout = pipelineLayout;

    m_pPipeline = new VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
    /*
    createTextureImage(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(),
                       TEXTURE_PATH, &textureImage, textureImageMemory,
                       m_Device.GetVkGraphicsQueue(), m_Device.GetVkCommandPool());

    textureImageView = createTextureImageView(m_Device.GetVkLogicalDevice(), textureImage);
    createTextureSampler(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(), textureSampler);
    */

    m_Model = new VulkanModel(m_Device, MODEL_PATH);
 
    createUniformBuffers(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(), uniformBuffers, uniformBuffersMemory);

    createDescriptorSets(m_Device.GetVkLogicalDevice(), m_Device.GetVkDescriptorPool(), descriptorSetLayout, descriptorSets, uniformBuffers, textureImageView, textureSampler);
}

Application::~Application() 
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(m_Device.GetVkLogicalDevice(), uniformBuffers[i], nullptr);
        vkFreeMemory(m_Device.GetVkLogicalDevice(), uniformBuffersMemory[i], nullptr);
    }


    vkDestroySampler(m_Device.GetVkLogicalDevice(), textureSampler, nullptr);
    vkDestroyImageView(m_Device.GetVkLogicalDevice(), textureImageView, nullptr);

    vkDestroyImage(m_Device.GetVkLogicalDevice(), textureImage, nullptr);
    vkFreeMemory(m_Device.GetVkLogicalDevice(), textureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(m_Device.GetVkLogicalDevice(), descriptorSetLayout, nullptr);
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

void Application::recreateSwapChain()
{
    auto extent = m_Window.GetVkExtent2D();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = m_Window.GetVkExtent2D();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_Device.GetVkLogicalDevice());


    VulkanSwapchain m_Swapchain{ m_Device, m_Window.GetVkExtent2D() };

    createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(PushConstantObject));

    VulkanPipelineSpecification pipelineSpec;
    pipelineSpec.vertFilepath   = "shaders/vert.spv";
    pipelineSpec.fragFilepath   = "shaders/frag.spv";
    pipelineSpec.pipelineLayout = pipelineLayout;

    m_pPipeline = new VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
}

void Application::drawFrame()
{
    auto currentFrame  = m_Renderer.currentFrame;
    auto currentBuffer = m_Renderer.GetCurrentBuffer();

    uint32_t imageIndex;
    VkResult result = m_Swapchain.AcquireNextImage(&imageIndex, currentFrame);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        //recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(m_Device.GetVkLogicalDevice(), currentFrame, m_Swapchain.GetVkExtent2D(), uniformBuffersMemory);

    vkResetCommandBuffer(currentBuffer, 0);

    recordCommandBuffer(currentBuffer);
    m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);
    m_pPipeline->bind(currentBuffer);
    bindDescriptorSet(currentBuffer, pipelineLayout, descriptorSets, currentFrame);
    m_Swapchain.BindDynamicViewport(currentBuffer);
    m_Model->bind(currentBuffer);

    /*
    for (int i = 0; i < 4; i++)
    {
        PushConstantObject pco{};
        pco.offset = {0.25f * i, 0.25f * i};
        pco.color  = {0.25f * i, 0.25f * i, 0.25f * i};
        bindPushConstant(currentBuffer, pipelineLayout, &pco);
        
    }
    */
    m_Model->draw(currentBuffer);

    m_Swapchain.EndRenderPass(currentBuffer);
    endCommandBuffer(currentBuffer);

    result = m_Swapchain.PresentQueue(currentBuffer, &imageIndex, currentFrame);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.getWindowSizeResized())
    {
        m_Window.setWindowSizeResized();
        //recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_Renderer.EndDraw();

}
