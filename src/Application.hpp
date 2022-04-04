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

    const uint32_t    WIDTH  = 800;
    const uint32_t    HEIGHT = 600;
    const std::string tiTLE  = "Myu Engine";

private:
    Window          m_Window{WIDTH, HEIGHT, "Test"};
    VulkanDevice    m_Device{m_Window.GetGLFWWindow()};
    VulkanSwapchain m_Swapchain{ m_Device, m_Window.GetVkExtent2D() };
    VulkanRenderer  m_Renderer{ m_Device, m_Window };

    VulkanPipeline* m_pPipeline;
    VkPipelineLayout m_PipelineLayout;

    void mainLoop();
    void cleanup();
    void drawFrame();

    void recreateSwapChain();
};