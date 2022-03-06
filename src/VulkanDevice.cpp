#include <VulkanDevice.hpp>

namespace VulkanWrapper
{
    void VulkanDevice::createPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // 실제 물리적 GPU를 탐색하고 갯수 반환

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // VkPhysicalDevice 목록을 가져옴

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface)) { // 만약에 적합하다면
                m_PhysicalDevice = device; // 바로 선택, 여러개의 물리적 GPU가 탐색된다면 성능 평가함수가 필요할 수 있음
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // 하나의 물리 디바이스에 여러개의 논리 디바이스가 존재할 수 있음, 이때 extension, queue family가 다를 수 있음
    void VulkanDevice::createLogicalDevice(const VkPhysicalDevice &physicalDevice, const VkSurfaceKHR &surface) {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // 필요한 Queue Family 찾음
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{}; // 몇 개의 Queue를 사용하는지
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1; // 보통 하나의 큐패밀리에서 하나의 큐를 사용함 -> 단일 큐에서 모든 명령 버퍼를 동시에 처리하기 때문
            queueCreateInfo.pQueuePriorities = &queuePriority; // 큐 우선순위
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{}; // will come back

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // queue family 갯수
        createInfo.pQueueCreateInfos = queueCreateInfos.data(); // queue family 추가

        createInfo.pEnabledFeatures = &deviceFeatures;

        // 기존에 VK_KHR_swapchain Extension이 추가되있는데, 장치에서 Draw된 이미지를 Window에서 표시하는데 사용됨
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size()); // extension 추가
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

        //if (vkContext->m_Debugger != nullptr) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size()); // layer 추가
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
        //}
        //else {
        //    createInfo.enabledLayerCount = 0;
        //}

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // Queue는 논리장치와 함께 생성되지만 따로 Handle을 담아야 함
        // 생성된 Queue는 논리장치와 함께 자동으로 파괴됨

        vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
    }
}