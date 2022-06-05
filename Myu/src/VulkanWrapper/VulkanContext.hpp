#pragma once

#include "../Core/Singleton.hpp"
#include <vulkan/vulkan.h>
#include "VulkanDevice.hpp"

namespace Myu::VulkanWrapper
{
    class VulkanContext : public Singleton<VulkanContext>
    {
    public:
        void init();
        void init(const VulkanDevice& device); //FIXME: will be deleted
        
        VkPhysicalDevice getPhysicalDevice() const {return mPhysicalDevice;}
        VkDevice getDevice() const {return mDevice;}
        uint32_t getGraphicsQueueIndex() const {return mQueueFamily.graphicsFamily.value();}
        VkCommandPool getCommandPool() const {return mCommandPool;}
        VkQueue getGraphicsQueue() const {return mGraphicsQueue;}
        
    private:
        VkPhysicalDevice mPhysicalDevice;
        VkDevice mDevice;
        QueueFamilyIndices mQueueFamily;
        VkCommandPool mCommandPool;
        VkQueue mGraphicsQueue;
    };
}
