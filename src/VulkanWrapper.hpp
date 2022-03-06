/*
    최상단에 선언하세요
    #include <VulkanWrapper.hpp>
*/

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <set>
#include <optional>

namespace VulkanWrapper
{
    static const std::vector<const char*> &m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
    static const std::vector<const char*> &m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    static struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily; // optional은 값이 할당될 떄 값을 가짐 
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    static struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities; // 이미지 해상도, 큐 사이즈 등
        std::vector<VkSurfaceFormatKHR> formats; // 픽셀 포맷(RGBA 등), 컬러 스페이스(32bpp 등) 
        std::vector<VkPresentModeKHR> presentModes; // 화면 전환 방식
    };

    // 물리장치가 Application에서 요구하는 Extension을 지원하는지 검사하는 함수
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr); // 물리장치에서 지원하는 확장의 수를 가져옴

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()); // 확장들을 벡터에 저장

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end()); // 요구하는 확장들을 가져옴

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName); // 일치하는 확장들을 제거
        }

        return requiredExtensions.empty(); // 만약 전부 비어있다면 모두 지원한다는 뜻
    }

    // 물리장치에서 VK_QUEUE_GRAPHICS_BIT를 지원하는 큐 패밀리의 인덱스를 가져오는 함수
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // 물리장치에서 지원하는 큐패밀리의 수를 가져옴

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // 큐 패밀리 데이터 가져옴

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // 만약 그래픽 기능을 지원한다면
                indices.graphicsFamily = i; // 인덱스 저장
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport); // Surface가 프레젠테이션 모드를 지원하는지 가져옴

            if (presentSupport) {
                indices.presentFamily = i; // 지원한다면 인덱스 저장
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices; // 그래픽 큐와 프레젠테이션 큐가 다를 수 있으나 비효율적 입니다.
    }

    // 물리장치가 화면에 표시하기위한 스왑체인 기능을 지원하는지 
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;
        
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); // 질의 함수들. 위와 비슷하게 동작함

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) { // presentMode : 스왑체인 내의 이미지를 화면과 스왑하는 방식 ( 삼중버퍼링, 즉시쓰기 등 )
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    // 큐 패밀리, 확장 등을 지원하는지 검사
    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static void popDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

}
