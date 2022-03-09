/*
    ����ü���� Surface�� ǥ�õǱ� ���� ��ٸ��� �̹��� ť �Դϴ�.

    ����ü���� ������ �̹� Surface�� Presentation�� ȭ���� ���ΰ�ħ�ϴ� �󵵿�
    ����ȭ �ϴ� �� �Դϴ�.

    �ش� ����� VK_KHR_swapchain Extension�� �����ؾ� �մϴ�.
*/

#include "VulkanSwapchain.hpp"
#include "VulkanInstance.hpp"

namespace VulkanWrapper
{
    // ������ �ȼ� ���˰� �÷� �����̽��� �����ϴ� ǥ���� ������ ��ȯ�մϴ�.
    VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return availableFormats[0];
    }

    // ������ ���������̼� ��带 �����ϴ��� Ȯ���ϰ� ��ȯ�մϴ�.
    VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
        for (const auto& availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // ������ �ػ󵵿� ť ����� ��
    VkExtent2D VulkanSwapchain::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent; // ���� gpu�� ������ �ػ󵵰� �ִٸ� �����մϴ�.
        }
        else { // ��ȣ�ϴ� �ػ󵵰� ���ٸ�
            int width, height;
            glfwGetFramebufferSize(window, &width, &height); // �������� �ػ󵵸� ��������

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            // gpu���� �����ϴ� �ִ� �� �ּ� �ػ� ������ ��ȯ�մϴ�.
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    VulkanSwapchain::VulkanSwapchain(GLFWwindow* window, const VkSurfaceKHR& surface)
    {
        auto physicalDevice = VulkanInstance::instance().m_Device->GetVkPhysicalDevice();
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
    }
}