#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"
#include "MouseListener.hpp"

#include <chrono>
#include "../VulkanWrapper/VulkanTexture.hpp"
#include "../VulkanWrapper/VulkanInitializer.hpp"
#include "../VulkanWrapper/VulkanContext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"



VkPipelineLayout      pipelineLayout;
VkPipeline            graphicsPipeline;

VkImage        textureImage;
VkDeviceMemory textureImageMemory;
VkImageView    textureImageView;
VkSampler      textureSampler;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
Myu::KeyboardListener keyboardListener {};
Myu::MouseListener mouseListener {};
Myu::Camera camera = Myu::Camera();

bool imGUIFlag = true;
VkCommandPool gImGUICommandPool;
std::vector<VkCommandBuffer> gImGUICommandBuffers;
std::vector<VkFramebuffer> gImGUIFrameBuffers;
VkRenderPass gImGUIRenderpass;
std::vector<VkImage> gMainImages;

std::vector<VkDeviceMemory> gMainMemories;
std::vector<VkImageView> gMainImageViews;
std::vector<VkFramebuffer> gMainFrameBuffers;
std::vector<VkDescriptorSet> gMainFrameDescSets;

static ImGui_ImplVulkanH_Window g_MainWindowData;

namespace Myu
{
    Application::Application()
    {
        setupVulkan();
        setupImGUI();
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

        auto mainRenderPass = m_Swapchain.GetVkRenderPass();
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = mainRenderPass;
        renderPassInfo.framebuffer       = gMainFrameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_Swapchain.GetVkExtent2D();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color        = {{0.3f, 0.3f, 0.3f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues    = clearValues.data();

        vkCmdBeginRenderPass(currentBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//        m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);

        // process keyboard event
        for (auto& go : gameObjects)
        {
            mouseListener.moveInPlaneXZ(m_Window.GetGLFWWindow(), deltaTime, go);
            keyboardListener.moveInPlaneXZ(m_Window.GetGLFWWindow(), deltaTime, go);
        }

        renderGameObjects(currentBuffer);

        m_Swapchain.EndRenderPass(currentBuffer);
        VulkanWrapper::endCommandBuffer(currentBuffer);
        
        // imGUI drawing
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            // imgui ui
            {
                ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
                flags |= ImGuiWindowFlags_NoDocking;
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::Begin("DockSpace Demo", 0, flags);
                ImGui::PopStyleVar();

                if (ImGui::BeginMenuBar())
                {
                    if(ImGui::MenuItem("File")) {}
                    
                    if(ImGui::MenuItem("Exit")) {}
                    
                    ImGui::EndMenuBar();
                }

                // dockspace submission
                ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

                ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
                ImGuiID dock_id_hierarchy = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
                ImGuiID dock_id_log = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
                ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);

                ImGui::DockBuilderDockWindow("Log", dock_id_log);
                ImGui::DockBuilderDockWindow("Hierarchy", dock_id_hierarchy);
                ImGui::DockBuilderDockWindow("Properties", dock_id_prop);
                ImGui::DockBuilderDockWindow("Scene", dock_main_id);
                ImGui::DockBuilderFinish(dockspace_id);
                
                ImGui::DockSpace(dockspace_id); // ?´ê²Œ ??? í–‰?˜ì„œ ë¶ˆë ¤?¼í•˜?”ê±¸ê¹??...
                ImGui::Begin("Hierarchy");

                static bool check = false;
                static bool exec_once = false;
                ImGui::Checkbox("Polygon mode", &check);

                if (check) {
                    if (!exec_once) {
                        std::cout << "on" << std::endl;
                        exec_once = true;
                    }

                    VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};
                    pipelineSpec.vertFilepath = "shaders/shader.vert.spv";
                    pipelineSpec.fragFilepath = "shaders/shader.frag.spv";
                    pipelineSpec.pipelineLayout = pipelineLayout;
                    pipelineSpec.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
                    // viewport info setup
                    VkViewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (float)m_Swapchain.GetVkExtent2D().width;
                    viewport.height = (float)m_Swapchain.GetVkExtent2D().height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    pipelineSpec.viewportInfo.pViewports = &viewport;

                    VkRect2D scissor{};
                    scissor.offset = { 0, 0 };
                    scissor.extent = m_Swapchain.GetVkExtent2D();
                    pipelineSpec.viewportInfo.pScissors = &scissor;

                    m_pNewPipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

                } else {
                    if (exec_once) {
                        std::cout << "off" << std::endl;
                        exec_once = false;
                    }

                    VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};
                    pipelineSpec.vertFilepath = "shaders/shader.vert.spv";
                    pipelineSpec.fragFilepath = "shaders/shader.frag.spv";
                    pipelineSpec.pipelineLayout = pipelineLayout;
                    pipelineSpec.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
                    // viewport info setup
                    VkViewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (float)m_Swapchain.GetVkExtent2D().width;
                    viewport.height = (float)m_Swapchain.GetVkExtent2D().height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    pipelineSpec.viewportInfo.pViewports = &viewport;

                    VkRect2D scissor{};
                    scissor.offset = { 0, 0 };
                    scissor.extent = m_Swapchain.GetVkExtent2D();
                    pipelineSpec.viewportInfo.pScissors = &scissor;

                    m_pNewPipe = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
                }
                    
                ImGui::End();

                ImGui::Begin("Log");
                ImGui::Text("Hello World!");
                ImGui::End();
                
                ImGui::Begin("Properties");
                ImGui::End();
                
                ImGui::Begin("Scene");
                auto scene_size = ImGui::GetWindowSize();
                ImGui::Image(gMainFrameDescSets[currentFrame], scene_size);
                ImGui::End();
                

                ImGui::End();
                ImGui::PopStyleVar();
        
            
                ImGui::Render();
            }
            
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            auto err = vkBeginCommandBuffer(gImGUICommandBuffers[currentFrame], &info);
            VK_CHECK_RESULT(err);
            
            VkClearValue clearValue;
            clearValue.color ={{0.3f, 0.3f, 0.3f, 1.0f}};
            VkRenderPassBeginInfo info2 = {};
            info2.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info2.renderPass = gImGUIRenderpass;
            info2.framebuffer = gImGUIFrameBuffers[imageIndex];
            info2.renderArea.extent.width = m_Swapchain.GetVkExtent2D().width;
            info2.renderArea.extent.height = m_Swapchain.GetVkExtent2D().height;
            info2.clearValueCount = 1;
            info2.pClearValues = &clearValue;
            vkCmdBeginRenderPass(gImGUICommandBuffers[currentFrame], &info2, VK_SUBPASS_CONTENTS_INLINE);
            
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), gImGUICommandBuffers[currentFrame]);
            
            vkCmdEndRenderPass(gImGUICommandBuffers[currentFrame]);
            err = vkEndCommandBuffer(gImGUICommandBuffers[currentFrame]);
            VK_CHECK_RESULT(err);
        }

        std::vector bufs{currentBuffer, gImGUICommandBuffers[currentFrame]};
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
        
        auto model2 = std::make_shared<Model>(m_Device, "models/paimon.obj");
        auto testGO2 = GameObject::createGameObject();
        testGO2.model = model2;
        testGO2.transform.position = glm::vec3(0.f, 0.f, 0.f);
        gameObjects.push_back(std::move(testGO2));
    }

    void Application::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        if (m_pNewPipe != nullptr) { // pipe swap
            m_pPipeline = m_pNewPipe;
            m_pNewPipe = nullptr;
        }
            
        m_pPipeline->bind(commandBuffer);
        
        for (auto& go : gameObjects)
        {
            // FIMXE: HACK
            go.model->bind(commandBuffer, pipelineLayout, go.transform.toMat4(), camera.getView(), camera.getProjection());
            
            go.model->draw(commandBuffer);
        }
    }

    void Application::setupVulkan()
    {
        // vulkan context initialize
        VulkanWrapper::VulkanContext::getInstance()->init(m_Device);
        
        loadGameObjects();
        
        //FIXME: HACK
        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &gameObjects[0].model->getMeshes()[0].getMaterial().getDescriptorLayout(), &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));
        
        VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};
        pipelineSpec.vertFilepath   = "shaders/shader.vert.spv";
        pipelineSpec.fragFilepath   = "shaders/shader.frag.spv";
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
        
        // camera setup
        camera.setViewTarget(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0));
        float aspect = m_Swapchain.GetVkExtent2D().width / (float)m_Swapchain.GetVkExtent2D().height;
        camera.setPerspectiveProjection(glm::radians(45.f), aspect, 0.1f, 100.f);
        
        // DescriptorPool Allocator Setup
        mDescAllocator.init(m_Device.GetVkLogicalDevice());
        
        // setup frame images
        auto imageCount = m_Swapchain.GetVkImages().size();
        gMainImages.resize(imageCount);
        gMainMemories.resize(imageCount);
        for (int i = 0; i < imageCount; i++)
        {
            VulkanWrapper::Utils::createImage(m_Swapchain.GetVkExtent2D().width, m_Swapchain.GetVkExtent2D().height, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gMainImages[i], &gMainMemories[i]);
            VulkanWrapper::Utils::transitionImageLayout(gMainImages[i], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        
        // setup image views
        gMainImageViews.resize(gMainImages.size());
        for (int i = 0; i < gMainImages.size(); i++)
        {
            VulkanWrapper::Utils::createImageView(gMainImages[i], &gMainImageViews[i], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        }
        
        // create temp depth resource
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VulkanWrapper::Utils::createImage(m_Swapchain.GetVkExtent2D().width, m_Swapchain.GetVkExtent2D().height,
                    VK_FORMAT_D32_SFLOAT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    &depthImage,
                    &depthImageMemory);
        
        VkImageView depthImageView;
        VulkanWrapper::Utils::createImageView(depthImage, &depthImageView, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
        
        gMainFrameBuffers.resize(gMainImageViews.size());
        for (size_t i = 0; i < gMainImageViews.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {gMainImageViews[i], depthImageView};
            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass      = m_Swapchain.GetVkRenderPass();
            frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferInfo.pAttachments    = attachments.data();
            frameBufferInfo.width           = m_Swapchain.GetVkExtent2D().width;
            frameBufferInfo.height          = m_Swapchain.GetVkExtent2D().height;
            frameBufferInfo.layers          = 1;
            
            if (vkCreateFramebuffer(m_Device.GetVkLogicalDevice(), &frameBufferInfo, nullptr,
                                    &gMainFrameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create VulkanFrameBuffer!");
            }
        }
    }
    
    // Only use for ImGUI
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    void Application::setupImGUI()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

         // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // create DescPool for only ImGUI
        auto imguiPool = VulkanWrapper::Init::createDefaultDescPool(m_Device.GetVkLogicalDevice());
        
        //Create renderpass dedicated to imGUI
        VkAttachmentDescription attachment = {};
        attachment.format = m_Swapchain.GetVkFormat();
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference attachmentRef = {};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRef;
        
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        if (vkCreateRenderPass(m_Device.GetVkLogicalDevice(), &info, nullptr, &gImGUIRenderpass) != VK_SUCCESS) {
            throw std::runtime_error("Could not create Dear ImGui's render pass");
        }
        
         // Setup Platform/Renderer backends
         ImGui_ImplGlfw_InitForVulkan(m_Window.GetGLFWWindow(), true);
         ImGui_ImplVulkan_InitInfo init_info = {};
         init_info.Instance = m_Device.GetVkInstance();
         init_info.PhysicalDevice = m_Device.GetVkPhysicalDevice();
         init_info.Device = m_Device.GetVkLogicalDevice();
         init_info.QueueFamily = m_Device.GetQueueFamilyIndices().graphicsFamily.value();
         init_info.Queue = m_Device.GetVkGraphicsQueue();
         init_info.PipelineCache = VK_NULL_HANDLE;
         init_info.DescriptorPool = imguiPool;
         init_info.MinImageCount = 2;
         init_info.ImageCount = m_Swapchain.GetVkImages().size();
         init_info.Allocator = nullptr;
         init_info.CheckVkResultFn = check_vk_result;
         ImGui_ImplVulkan_Init(&init_info, gImGUIRenderpass);
        
        // setup default font texture
        VkCommandBuffer command_buffer = VulkanWrapper::Utils::beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        VulkanWrapper::Utils::endSingleTimeCommands(command_buffer);
        
        //setup command buffers
        VulkanWrapper::Init::createCommandPool(&gImGUICommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        gImGUICommandBuffers.resize(m_Swapchain.GetVkImages().size());
        VulkanWrapper::Init::createCommandBuffers(gImGUICommandBuffers.data(), static_cast<uint32_t>(gImGUICommandBuffers.size()), gImGUICommandPool);
        
        // setup frame buffers
        gImGUIFrameBuffers.resize(m_Swapchain.GetImageViews().size());
        for (size_t i = 0; i < m_Swapchain.GetImageViews().size(); i++)
        {
            std::array<VkImageView, 1> attachments = {m_Swapchain.GetImageViews()[i]};
            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass      = gImGUIRenderpass;
            frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferInfo.pAttachments    = attachments.data();
            frameBufferInfo.width           = m_Swapchain.GetVkExtent2D().width;
            frameBufferInfo.height          = m_Swapchain.GetVkExtent2D().height;
            frameBufferInfo.layers          = 1;
            
            if (vkCreateFramebuffer(m_Device.GetVkLogicalDevice(), &frameBufferInfo, nullptr,
                                    &gImGUIFrameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create VulkanFrameBuffer!");
            }
        }
        
        // get maxSamplerAnisotropy for sampler
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.GetVkPhysicalDevice(), &properties);
        
        // create sampler
        VkSampler sampler;
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VK_CHECK_RESULT(vkCreateSampler(m_Device.GetVkLogicalDevice(), &samplerInfo, nullptr, &sampler));
        
        gMainFrameDescSets.resize(gMainImageViews.size());
        for (int i = 0; i < gMainImageViews.size(); i++)
        {
            gMainFrameDescSets[i] = ImGui_ImplVulkan_AddTexture(sampler, gMainImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
}
