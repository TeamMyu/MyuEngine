#pragma once

#include "VulkanUtils.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct VulkanCommandBuffersSpecification
    {
        uint32_t queueFamilyIndex;
        uint32_t bufferCount;
    };

    class VulkanCommandBuffers
    {
    public:
        VulkanCommandBuffers(const VulkanCommandBuffersSpecification &spec);
        ~VulkanCommandBuffers();

        std::vector<VkCommandBuffer> GetVkCommandBuffers() {return m_VkCommandBuffers;}

    private:
        std::vector<VkCommandBuffer> m_VkCommandBuffers;
        VkCommandPool m_VkCommandPool;
    };
}
