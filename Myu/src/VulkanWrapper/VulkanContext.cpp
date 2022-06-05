#include "VulkanContext.hpp"

namespace Myu::VulkanWrapper
{
    void VulkanContext::init(const VulkanDevice& device)
    {
        mPhysicalDevice = device.GetVkPhysicalDevice();
        mDevice = device.GetVkLogicalDevice();
        mQueueFamily = device.GetQueueFamilyIndices();
        mCommandPool = device.GetVkCommandPool();
        mGraphicsQueue = device.GetVkGraphicsQueue();
    }

    void VulkanContext::init()
    {
        
    }
}

