#pragma once

#include "Vulkan.hpp"

namespace Myu::VulkanWrapper
{
#ifdef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = true;
#endif

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    class VulkanDevice
    {
    public:
        VulkanDevice(GLFWwindow* window);
        ~VulkanDevice();

        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice operator=(const VulkanDevice&) = delete;

        VkSurfaceKHR            GetSurface() { return m_Surface; }
        VkInstance              GetVkInstance() { return m_Instance; }
        VkPhysicalDevice        GetVkPhysicalDevice() { return m_PhysicalDevice; }
        VkDevice                GetVkLogicalDevice() { return m_Device; }
        VkQueue                 GetVkGraphicsQueue() { return m_GraphicsQueue; }
        VkQueue                 GetVkPresentQueue() { return m_PresentQueue; }
        QueueFamilyIndices      GetQueueFamilyIndices() { return findQueueFamilies(m_PhysicalDevice); }
        SwapChainSupportDetails GetSwapChainDetails() { return querySwapChainSupport(m_PhysicalDevice); }
        VkCommandPool           GetVkCommandPool() { return m_CommandPool; }
        VkDescriptorPool        GetVkDescriptorPool() { return m_DescriptorPool; }

        std::vector<VkCommandBuffer> GetCommandBuffers() { return m_CommandBuffers; }

    private:
        const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
        // Class Ref
        GLFWwindow* m_rWindow;
        // ---
        VkSurfaceKHR                 m_Surface;
        VkInstance                   m_Instance;
        VkPhysicalDevice             m_PhysicalDevice;
        VkDevice                     m_Device;
        VkQueue                      m_GraphicsQueue;
        VkQueue                      m_PresentQueue;
        QueueFamilyIndices           m_QueueFamilyIndices;
        VkDebugUtilsMessengerEXT     m_DebugMessenger;
        VkCommandPool                m_CommandPool;
        VkDescriptorPool             m_DescriptorPool;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        void createSurface();
        void createInstance();
        void createPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
        void createDescriptorPool();
        void createCommandBuffers();

        void setupDebugMessenger();

        VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks*              pAllocator,
                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger);
        void     destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT     debugMessenger,
                                               const VkAllocationCallbacks* pAllocator);

        std::vector<const char*> getRequiredExtensions();

        bool                    checkValidationLayerSupport();
        bool                    checkDeviceExtensionSupport(VkPhysicalDevice device);
        bool                    isDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device);
        uint32_t                findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        void popDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    };
}  // namespace VulkanWrapper
