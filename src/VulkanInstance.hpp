#pragma once

#include "Vulkan.hpp"
#include "VulkanDevice.hpp"
#include "VulkanDebug.hpp"

namespace VulkanWrapper
{
    class VulkanInstance
    {
    public:
        VulkanDebug* m_Debugger;
        VulkanDevice* m_Device;

        static VulkanInstance& instance()
        {
            static VulkanInstance* _inst = new VulkanInstance();
            return *_inst;
        }

        VkInstance GetVkInstance() { return m_VkInstance; }

        std::vector<const char*> getRequiredExtensions();

    private:
        VkInstance m_VkInstance;

        VulkanInstance();
    };
}



