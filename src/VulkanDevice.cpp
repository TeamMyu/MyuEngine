#include "VulkanDevice.hpp"
#include "VulkanInstance.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    void VulkanDevice::createPhysicalDevice(const VkSurfaceKHR& surface)
    {
        auto instance = VulkanInstance::instance().GetVkInstance();
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // ���� ������ GPU�� Ž���ϰ� ���� ��ȯ

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // VkPhysicalDevice ����� ������

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface)) { // ���࿡ �����ϴٸ�
                m_PhysicalDevice = device; // �ٷ� ����, �������� ������ GPU�� Ž���ȴٸ� ���� ���Լ��� �ʿ��� �� ����
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // �ϳ��� ���� ����̽��� �������� ���� ����̽��� ������ �� ����, �̶� extension, queue family�� �ٸ� �� ����
    void VulkanDevice::createLogicalDevice(const VkSurfaceKHR &surface) {
        m_indices = findQueueFamilies(m_PhysicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // �ʿ��� Queue Family ã��
        std::set<uint32_t> uniqueQueueFamilies = { m_indices.graphicsFamily.value(), m_indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{}; // �� ���� Queue�� ����ϴ���
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1; // ���� �ϳ��� ť�йи����� �ϳ��� ť�� ����� -> ���� ť���� ��� ���� ���۸� ���ÿ� ó���ϱ� ����
            queueCreateInfo.pQueuePriorities = &queuePriority; // ť �켱����
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{}; // will come back

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // queue family ����
        createInfo.pQueueCreateInfos = queueCreateInfos.data(); // queue family �߰�

        createInfo.pEnabledFeatures = &deviceFeatures;

        // ������ VK_KHR_swapchain Extension�� �߰����ִµ�, ��ġ���� Draw�� �̹����� Window���� ǥ���ϴµ� ����
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size()); // extension �߰�
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

        //if (vkContext->m_Debugger != nullptr) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size()); // layer �߰�
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
        //}
        //else {
        //    createInfo.enabledLayerCount = 0;
        //}

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // Queue�� ������ġ�� �Բ� ���������� ���� Handle�� ��ƾ� ��
        // ������ Queue�� ������ġ�� �Բ� �ڵ����� �ı���

        vkGetDeviceQueue(m_Device, m_indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_indices.presentFamily.value(), 0, &m_PresentQueue);
    }

    VulkanDevice::VulkanDevice(const VkSurfaceKHR& surface)
    {
        createPhysicalDevice(surface);
        createLogicalDevice(surface);
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDestroyDevice(m_Device, nullptr);
    }
}