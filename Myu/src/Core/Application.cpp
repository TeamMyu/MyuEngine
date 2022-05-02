#include "Application.hpp"
#include "Camera.hpp"
#include "KeyboardListener.hpp"

#include <chrono>
#include "Debug.hpp"

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
    void Application::createCommandPool(VkCommandPool* cmdPool)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_Device.GetQueueFamilyIndices().graphicsFamily.value();

        if (vkCreateCommandPool(m_Device.GetVkLogicalDevice(), &poolInfo, nullptr, cmdPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void Application::createImGuiFramebuffers()
    {
        m_ImGuiFramebuffers.resize(m_Swapchain.GetImageViews().size());

        VkImageView             attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass              = m_ImGuiRenderPass;
        info.attachmentCount         = 1;
        info.pAttachments            = attachment;
        info.width                   = m_Swapchain.GetVkExtent2D().width;
        info.height                  = m_Swapchain.GetVkExtent2D().height;
        info.layers                  = 1;
        for (uint32_t i = 0; i < m_Swapchain.GetImageViews().size(); i++)
        {
            attachment[0] = m_Swapchain.GetImageViews()[i];
            if (vkCreateFramebuffer(m_Device.GetVkLogicalDevice(), &info, nullptr, &m_ImGuiFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void Application::createImGuiCommandBuffers()
    {
        m_ImGuiCommandBuffers.resize(m_Swapchain.GetImageViews().size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = m_ImGuiCommandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_ImGuiCommandBuffers.size();

        if (vkAllocateCommandBuffers(m_Device.GetVkLogicalDevice(), &allocInfo, m_ImGuiCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    void Application::createImGuiRenderPass()
    {
        VkAttachmentDescription attachment = {};
        attachment.format                  = m_Swapchain.GetVkFormat();
        attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment = {};
        color_attachment.attachment            = 0;
        color_attachment.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &color_attachment;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask       = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info = {};
        info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount        = 1;
        info.pAttachments           = &attachment;
        info.subpassCount           = 1;
        info.pSubpasses             = &subpass;
        info.dependencyCount        = 1;
        info.pDependencies          = &dependency;

        if (vkCreateRenderPass(m_Device.GetVkLogicalDevice(), &info, nullptr, &m_ImGuiRenderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }
    void Application::initEditor()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        {
            createImGuiRenderPass();
        }

        // Create CommandPool for m_ImGuiCommandPool
        {
            createCommandPool(&m_ImGuiCommandPool);
        }

        // Create CommandBuffers for m_ImGuiCommandBuffers
        {
            createImGuiCommandBuffers();
        }

        {
            createImGuiFramebuffers();
        }
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
            };

            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets                    = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount              = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes                 = pool_sizes;

            if (vkCreateDescriptorPool(m_Device.GetVkLogicalDevice(), &pool_info, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
                throw std::runtime_error("Create DescriptorPool for m_ImGuiDescriptorPool failed!");
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(m_Window.GetGLFWWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = m_Device.GetVkInstance();
        init_info.PhysicalDevice            = m_Device.GetVkPhysicalDevice();
        init_info.Device                    = m_Device.GetVkLogicalDevice();
        init_info.QueueFamily               = m_Device.GetQueueFamilyIndices().graphicsFamily.value();
        init_info.Queue                     = m_Device.GetVkGraphicsQueue();
        init_info.PipelineCache             = nullptr;
        init_info.DescriptorPool            = m_ImGuiDescriptorPool;
        init_info.Subpass                   = 0;
        init_info.MinImageCount             = VulkanWrapper::MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount                = m_Swapchain.GetVkImages().size();
        init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator                 = nullptr;
        init_info.CheckVkResultFn           = nullptr;
        ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

        // Upload Fonts
        {
            VkCommandBuffer commandBuffer = VulkanWrapper::beginSingleTimeCommands(m_Device.GetVkLogicalDevice(), m_Device.GetVkCommandPool());
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            VulkanWrapper::endSingleTimeCommands(m_Device.GetVkLogicalDevice(), commandBuffer, m_Device.GetVkGraphicsQueue(), m_Device.GetVkCommandPool());
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void Application::drawEditor()
    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // hierarchy
        {
            ImGui::Begin("hierarchy", nullptr);  // Create a window called "Hello, world!" and append into it.
            ImGui::SetWindowSize(ImVec2((1280 - 800) / 2, 600));
            ImGui::SetWindowPos(ImVec2(0, 0));
            ImGui::End();
        }

        // viewport
        {
            //VkDescriptorSet m_Dset = ImGui_ImplVulkan_AddTexture(textureSampler, m_Swapchain.get, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            ImGui::Begin("viewport", nullptr);  // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::SetWindowSize(ImVec2(800, 600));
            ImGui::SetWindowPos(ImVec2((1280 - 800) / 2, 0));
            ImGui::Button("Play");
            ImGui::Button("Stop");
            ImGui::End();
        }

        // console
        {
            ImGui::Begin("console", nullptr);  // Create a window called "Hello, world!" and append into it.
            ImGui::SetWindowSize(ImVec2(1280 - ((1280 - 800) / 2), 800 - 600));
            ImGui::SetWindowPos(ImVec2(0, 600));

            ImGui::End();
        }

        // inspector
        {
            ImGui::Begin("inspector", nullptr);  // Create a window called "Hello, world!" and append into it.
            ImGui::SetWindowSize(ImVec2((1280 - 800) / 2, 800));
            ImGui::SetWindowPos(ImVec2((1280 / 2 + 400), 0));
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
    }

    Application::Application()
    {
        loadGameObjects();
        
        // creatae descriptor set layout
        auto uniformBinding = VulkanWrapper::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
        std::vector<VkDescriptorSetLayoutBinding> bindings { uniformBinding };
        VulkanWrapper::createDescriptorSetLayout(m_Device.GetVkLogicalDevice(), bindings, &descriptorSetLayout);
        
        // create descriptor set
        for (auto& go : gameObjects)
        {
            VulkanWrapper::createUniformDescriptorSet(m_Device.GetVkLogicalDevice(), m_Device.GetVkDescriptorPool(), descriptorSetLayout, go.model->getDescriptorSet(), go.model->getUniformBuffer());
        }
        
        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));

        VulkanWrapper::VulkanPipelineSpecification pipelineSpec{};
        pipelineSpec.vertFilepath   = "shaders/vert.spv";
        pipelineSpec.fragFilepath   = "shaders/frag.spv";
        pipelineSpec.pipelineLayout = pipelineLayout;
        m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
        
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
//        initEditor();
        mainLoop();
    }

    void Application::mainLoop()
    {
        while (!glfwWindowShouldClose(m_Window.GetGLFWWindow()))
        {
            glfwPollEvents();
//            drawEditor();
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

        VulkanWrapper::VulkanSwapchain m_Swapchain{m_Device, m_Window.GetVkExtent2D()};

        VulkanWrapper::createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(VulkanWrapper::PushConstantObject));

        VulkanWrapper::VulkanPipelineSpecification pipelineSpec;
        pipelineSpec.vertFilepath   = "shaders/vert.spv";
        pipelineSpec.fragFilepath   = "shaders/frag.spv";
        pipelineSpec.pipelineLayout = pipelineLayout;

        m_pPipeline = new VulkanWrapper::VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
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
        result = m_Swapchain.PresentQueue(bufs, &imageIndex, currentFrame);
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
        
        auto model2 = std::make_shared<Model>(m_Device, "models/smooth_vase.obj");
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
