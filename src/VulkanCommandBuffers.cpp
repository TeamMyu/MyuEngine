#include "VulkanCommandBuffers.hpp"
#include "VulkanInstance.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    VulkanCommandBuffers::VulkanCommandBuffers(const VulkanCommandBuffersSpecification& spec)
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = spec.queueFamilyIndex;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_VkCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = spec.bufferCount;

        if (vkAllocateCommandBuffers(device, &allocInfo, m_VkCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    VulkanCommandBuffers::~VulkanCommandBuffers()
    {
        auto device = VulkanInstance::instance().m_Device->GetVkLogicalDevice();
        vkDestroyCommandPool(device, m_VkCommandPool, nullptr);
    }
}
