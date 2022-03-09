#pragma once

#include "Vulkan.hpp"

#include <vector>

namespace VulkanWrapper
{
    struct CommandBuffersSpecification
    {
        VkDevice device;
        uint32_t queueFamilyIndex;
        uint32_t bufferCount;
    };

    class CommandBuffers
    {
    public:
        CommandBuffers(const CommandBuffersSpecification &spec);
        ~CommandBuffers();

        VkCommandBuffer GetVkCommandBuffer(int idx) {return m_VkCommandBuffers[idx];}

        void Begin(uint32_t idx);
        void End(uint32_t idx);

    private:
        std::vector<VkCommandBuffer> m_VkCommandBuffers;
        CommandBuffersSpecification m_Specification;
        VkCommandPool m_VkCommandPool;
    };
}
