

#include "Application.hpp"
#include "Camera.hpp"

VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout      pipelineLayout;
VkPipeline            graphicsPipeline;

VkImage        textureImage;
VkDeviceMemory textureImageMemory;
VkImageView    textureImageView;
VkSampler      textureSampler;

std::vector<VkBuffer>       uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;

std::vector<VkDescriptorSet> descriptorSets;

const std::string MODEL_PATH   = "resources/models/viking_room.obj";
const std::string TEXTURE_PATH = "resources/textures/viking_room.png";

std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
VkDescriptorPool             m_ImGuiDescriptorPool;
VkCommandPool                m_ImGuiCommandPool;
std::vector<VkFramebuffer>   m_ImGuiFramebuffers;
VkRenderPass                 m_ImGuiRenderPass;

VulkanModel *m_Model;

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
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

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
    init_info.MinImageCount             = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount                = m_Swapchain.GetVkImages().size();
    init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator                 = nullptr;
    init_info.CheckVkResultFn           = nullptr;
    ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

    // Upload Fonts
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_Device.GetVkLogicalDevice(), m_Device.GetVkCommandPool());
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        endSingleTimeCommands(m_Device.GetVkLogicalDevice(), commandBuffer, m_Device.GetVkGraphicsQueue(), m_Device.GetVkCommandPool());
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

void Application::drawEditor()
{
 
}


Application::Application()
{
    createDescriptorSetLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout);
    createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(PushConstantObject));

    VulkanPipelineSpecification pipelineSpec;
    pipelineSpec.vertFilepath   = "resources/shaders/vert.spv";
    pipelineSpec.fragFilepath   = "resources/shaders/frag.spv";
    pipelineSpec.pipelineLayout = pipelineLayout;

    m_pPipeline = new VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);

    //m_ViewportSwapchain = new VulkanSwapchain(m_Device, m_Window.GetVkExtent2D());
    //m_pViewportPipeline = new VulkanPipeline(m_Device, m_ViewportSwapchain->GetVkRenderPass(), pipelineSpec);
    /*
    createTextureImage(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(),
                       TEXTURE_PATH, &textureImage, textureImageMemory,
                       m_Device.GetVkGraphicsQueue(), m_Device.GetVkCommandPool());

    textureImageView = createTextureImageView(m_Device.GetVkLogicalDevice(), textureImage);
    */
    createTextureSampler(m_Device.GetVkPhysicalDevice(), m_Device.GetVkLogicalDevice(), textureSampler);
    

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

    glfwTerminate();
}

void Application::run()
{
    initEditor();
    mainLoop();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(m_Window.GetGLFWWindow()))
    {
        glfwPollEvents();
        drawEditor();
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

    VulkanSwapchain m_Swapchain{m_Device, m_Window.GetVkExtent2D()};

    createPipelineLayout(m_Device.GetVkLogicalDevice(), &descriptorSetLayout, &pipelineLayout, sizeof(PushConstantObject));

    VulkanPipelineSpecification pipelineSpec;
    pipelineSpec.vertFilepath   = "resources/shaders/vert.spv";
    pipelineSpec.fragFilepath   = "resources/shaders/frag.spv";
    pipelineSpec.pipelineLayout = pipelineLayout;

    m_pPipeline = new VulkanPipeline(m_Device, m_Swapchain.GetVkRenderPass(), pipelineSpec);
}

void Application::drawFrame()
{
    auto currentFrame  = m_Renderer.currentFrame;
    auto currentBuffer = m_Renderer.GetCurrentBuffer();

       glfwPollEvents();

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

    Myu::Camera cam = Myu::Camera();
    cam.setViewTarget(glm::vec3(3, -2, 0), glm::vec3(0, 0, 0));
    float aspect = m_Swapchain.GetVkExtent2D().width / (float)m_Swapchain.GetVkExtent2D().height;
    cam.setPerspectiveProjection(glm::radians(45.0f), aspect, 0.1f, 10.f);

    updateUniformBuffer(m_Device.GetVkLogicalDevice(), currentFrame, m_Swapchain.GetVkExtent2D(), uniformBuffersMemory, glm::mat4(1.0f), cam.getView(), cam.getProjection());

    vkResetCommandBuffer(currentBuffer, 0);

    recordCommandBuffer(currentBuffer);
    m_Swapchain.BeginRenderPass(currentBuffer, imageIndex);
    m_pPipeline->bind(currentBuffer);
    bindDescriptorSet(currentBuffer, pipelineLayout, descriptorSets, currentFrame);
    m_Swapchain.BindDynamicViewport(currentBuffer, {(1280 - 800) / 2, 0, 800, 600, 0, 0});
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

    {
    
        VkCommandBufferBeginInfo info = {};
        info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(m_ImGuiCommandBuffers[currentFrame], &info);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = m_ImGuiRenderPass;
        renderPassInfo.framebuffer           = m_ImGuiFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = m_Swapchain.GetVkExtent2D();
        VkClearValue clearColor              = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount       = 1;
        renderPassInfo.pClearValues          = &clearColor;
        vkCmdBeginRenderPass(m_ImGuiCommandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImGuiCommandBuffers[currentFrame]);

        vkCmdEndRenderPass(m_ImGuiCommandBuffers[currentFrame]);
        vkEndCommandBuffer(m_ImGuiCommandBuffers[currentFrame]);
    }

    std::vector bufs{currentBuffer, m_ImGuiCommandBuffers[currentFrame]};

    result = m_Swapchain.PresentQueue(bufs, &imageIndex, currentFrame);

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