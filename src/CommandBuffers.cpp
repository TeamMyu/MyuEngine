#include "CommandBuffers.hpp"

#include <stdexcept>

namespace VulkanWrapper
{
    CommandBuffers::CommandBuffers(const CommandBuffersSpecification& spec)
        : m_Specification(spec)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = spec.queueFamilyIndex;

        if (vkCreateCommandPool(spec.device, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_VkCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = spec.bufferCount;

        if (vkAllocateCommandBuffers(spec.device, &allocInfo, m_VkCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    CommandBuffers::~CommandBuffers()
    {
        vkDestroyCommandPool(m_Specification.device, m_VkCommandPool, nullptr);
    }

    void CommandBuffers::Begin(uint32_t idx)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(m_VkCommandBuffers[idx], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    void CommandBuffers::End(uint32_t idx)
    {
        if (vkEndCommandBuffer(m_VkCommandBuffers[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}
