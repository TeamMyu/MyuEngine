#pragma once

#include "Vulkan.hpp"
#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanModel.hpp"
#include "Window.hpp"

#include <vector>

using namespace VulkanWrapper;
using namespace MyuEngine;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application operator=(const Application&) = delete;

    void run();

    const uint32_t    WIDTH  = 1280;
    const uint32_t    HEIGHT = 800;
    const std::string tiTLE  = "Myu Engine";

    Window          m_Window{WIDTH, HEIGHT, "Test"};
    VulkanDevice    m_Device{m_Window.GetGLFWWindow()};
    VulkanSwapchain m_Swapchain{m_Device, m_Window.GetVkExtent2D()};
    VulkanRenderer  m_Renderer{m_Device, m_Window};

    VkImage        textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView    textureImageView;
    VkSampler      textureSampler;

    void createCommandPool(VkCommandPool* cmdPool);
    void createImGuiCommandBuffers();
    void createImGuiFramebuffers();
    void createImGuiRenderPass();
    void initEditor();
    void drawEditor();
    void drawFrame();

private:

    VulkanPipeline* m_pPipeline;
    VkPipelineLayout m_PipelineLayout;

    VulkanPipeline* m_pViewportPipeline;
    

    void mainLoop();
    void cleanup();
    

    void recreateSwapChain();
};