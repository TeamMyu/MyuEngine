#pragma once

#include "../VulkanWrapper/Vulkan.hpp"
#include "../VulkanWrapper/VulkanDevice.hpp"
#include "../VulkanWrapper/VulkanPipeline.hpp"
#include "../VulkanWrapper/VulkanSwapchain.hpp"
#include "Window.hpp"

#include "Model.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Core.hpp"

#include <vector>

namespace Myu
{
    class MYU_API Application
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
        VulkanWrapper::VulkanDevice    m_Device{m_Window.GetGLFWWindow()};
        VulkanWrapper::VulkanSwapchain m_Swapchain{m_Device, m_Window.GetVkExtent2D()};
        Renderer  m_Renderer{m_Device};

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

        static Application* CreateApplication()
        {
            return new Myu::Application();
        }

    private:

        VulkanWrapper::VulkanPipeline* m_pPipeline;
        VkPipelineLayout m_PipelineLayout;
        
        std::vector<GameObject> gameObjects;

        void mainLoop();
        void cleanup();
        
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);
    };
}
