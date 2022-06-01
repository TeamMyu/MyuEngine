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

        VkSurfaceKHR            GetSurface() const { return m_Surface; }
        VkInstance              GetVkInstance() const { return m_Instance; }
        VkPhysicalDevice        GetVkPhysicalDevice() const { return m_PhysicalDevice; }
        VkDevice                GetVkLogicalDevice() const { return m_Device; }
        VkQueue                 GetVkGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue                 GetVkPresentQueue() const { return m_PresentQueue; }
        VkQueue                 GetVkComputeQueue() const { return m_ComputeQueue; }
        QueueFamilyIndices      GetQueueFamilyIndices() { return findQueueFamilies(m_PhysicalDevice); }
        SwapChainSupportDetails GetSwapChainDetails() { return querySwapChainSupport(m_PhysicalDevice); }
        VkCommandPool           GetVkCommandPool() const { return m_CommandPool; }
        VkDescriptorPool        GetVkDescriptorPool() const { return m_DescriptorPool; }

        std::vector<VkCommandBuffer> GetCommandBuffers() { return m_CommandBuffers; }

        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);

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
        VkQueue                      m_ComputeQueue;
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
