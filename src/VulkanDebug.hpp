#pragma once

#include "VulkanUtils.hpp"
#include "VulkanInstance.hpp"

#include <iostream>
#include <vector>

namespace VulkanWrapper
{
    class VulkanDebug
    {
    public:
        VulkanDebug();
        ~VulkanDebug();

        VkDebugUtilsMessengerEXT GetDebugger() { return m_DebugMessenger; }

    private:
        VkDebugUtilsMessengerEXT m_DebugMessenger;

        void setupDebugMessenger();

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);

        void DestroyDebugUtilsMessengerEXT(VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

        bool checkValidationLayerSupport();
    };
}
