#include "VulkanUtils.hpp"
#include "VulkanDebug.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanImageView.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanCommandBuffers.hpp"
#include "VulkanRenderer.hpp"

#include "Window.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    VulkanWrapper::VulkanDebug *debugger;

    Myu::Window *window;

    VulkanWrapper::VulkanDevice* device;

    VulkanWrapper::VulkanSwapchain* swapChain;

    VulkanWrapper::VulkanImageView* imageView;

    VulkanWrapper::VulkanRenderPass* renderPass;

    VulkanWrapper::VulkanPipeline* graphicsPipeline;

    std::vector<VulkanWrapper::VulkanFrameBuffer*> frameBuffers;

    VulkanWrapper::VulkanCommandBuffers* commandBuffers;

    VulkanWrapper::VulkanRenderer* renderer;

    uint32_t currentFrame = 0;

    void initWindow()
    {
        window = new Myu::Window(WIDTH, HEIGHT, "Vulkan");
    }

    void initVulkan()
    {

        // validation layeer setup
        if (enableValidationLayers)
            debugger = new VulkanWrapper::VulkanDebug();

        window->createSurface();

        // create Physical & Logical device
        device = new VulkanWrapper::VulkanDevice(window->GetSurface());
        VulkanWrapper::VulkanInstance::instance().m_Device = device;

        swapChain = new VulkanWrapper::VulkanSwapchain({ window->GetGLFWWindow(), window->GetSurface() });

        imageView = new VulkanWrapper::VulkanImageView({swapChain->GetVkImages(), swapChain->GetVkFormat(), swapChain->GetVkExtent2D()});

        createRenderPass();

        graphicsPipeline = new VulkanWrapper::VulkanPipeline({renderPass->GetVkRenderPass(), swapChain->GetVkExtent2D()});

        createFramebuffers();

        commandBuffers = new VulkanWrapper::VulkanCommandBuffers({device->GetQueueFamilyIndices().graphicsFamily.value(), MAX_FRAMES_IN_FLIGHT});

        renderer = new VulkanWrapper::VulkanRenderer({MAX_FRAMES_IN_FLIGHT});
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window->GetGLFWWindow()))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device->GetVkLogicalDevice());
    }

    void cleanup()
    {
        delete renderer;

        delete commandBuffers;

        for (auto framebuffer : frameBuffers)
            delete framebuffer;

        delete graphicsPipeline;

        delete renderPass;

        delete imageView;

        delete swapChain;

        if (enableValidationLayers)
            delete debugger;

        delete device;

        delete window;
    }

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain->GetVkFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        std::vector<VkAttachmentDescription> attachments = {colorAttachment};
        std::vector<VkSubpassDescription> subpasses = {subpass};
        renderPass = new VulkanWrapper::VulkanRenderPass({swapChain->GetVkFormat(), subpasses, attachments});
        
    }

    void createFramebuffers()
    {
        auto vkImgViews = imageView->GetImageViews();

        for (size_t i = 0; i < vkImgViews.size(); i++)
        {
            frameBuffers.push_back(new VulkanWrapper::VulkanFrameBuffer({vkImgViews[i], renderPass->GetVkRenderPass(),
                                                                         swapChain->GetVkExtent2D()}));
        }
    }

    void drawFrame()
    {
        VulkanWrapper::DrawTriangleInfo info;
        info.commandBuffers = commandBuffers->GetVkCommandBuffers();
        info.extent = swapChain->GetVkExtent2D();
        info.frameBuffer = frameBuffers[currentFrame]->GetVkFrameBuffer();
        info.frameIndex = currentFrame;
        info.graphicsQueue = device->GetVkGraphicsQueue();
        info.pipeline = graphicsPipeline->GetVulkanPipeline();
        info.presentQueue = device->GetVkPresentQueue();
        info.renderpass = renderPass->GetVkRenderPass();
        info.swapchain = swapChain->GetVkSwapChain();

        renderer->DrawTriangle(info);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
