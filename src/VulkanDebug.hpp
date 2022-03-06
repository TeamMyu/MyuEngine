#pragma once

#include <VulkanWrapper.hpp>

namespace VulkanWrapper
{
    class VulkanDebug
    {
    public:
        void setupDebugMessenger(const VkInstance& instance);

        /* 캡슐화 불가능 및 변경가능으로 인해 버그발생 여지 있음
        inline const auto& getInstance() const { return m_Instance; }
        inline const auto& getDebugger()  const { return m_DebugMessenger; }
        반환값 수정x 참조형으로 전달 -> 받을때 참조형으로 받아야함
        */
        VkInstance m_Instance { VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT m_DebugMessenger { VK_NULL_HANDLE };

    private:
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