#pragma once

#include "Vulkan.hpp"
#include "VulkanDebug.hpp"
#include "VulkanDevice.hpp"

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

        VulkanDevice GetDevice() { return *m_Device; }

        VkInstance GetVkInstance() { return m_VkInstance; }

        std::vector<const char*> getRequiredExtensions();

    private:
        VkInstance m_VkInstance;
        VulkanDevice* m_Device;

        VulkanInstance();
    };
}



