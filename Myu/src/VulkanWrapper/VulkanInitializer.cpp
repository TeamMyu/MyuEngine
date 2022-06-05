#include "VulkanInitializer.hpp"
#include "VulkanContext.hpp"

#include <vector>

namespace Myu::VulkanWrapper::Init
{
    VkDescriptorPool createDefaultDescPool(VkDevice device)
    {
        int count = 1000;
        std::vector<std::pair<VkDescriptorType,float>> defaultSizes =
        {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 100.0f},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100.0f},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100.0f},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100.0f},
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100.0f}
        };
        
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(defaultSizes.size());
        for (auto sz : defaultSizes) {
            sizes.push_back({ sz.first, uint32_t(sz.second * count) });
        }
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = count;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes = sizes.data();

        VkDescriptorPool descriptorPool;
        vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

        return descriptorPool;
    }

    void createCommandPool(VkCommandPool* pCommandPool, VkCommandPoolCreateFlags flags) {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = VulkanContext::getInstance()->getGraphicsQueueIndex();
        commandPoolCreateInfo.flags = flags;

        if (vkCreateCommandPool(VulkanContext::getInstance()->getDevice(), &commandPoolCreateInfo, nullptr, pCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Could not create graphics command pool");
        }
    }

    void createCommandBuffers(VkCommandBuffer* pCommandBuffer, uint32_t commandBufferCount, VkCommandPool &commandPool) {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
        vkAllocateCommandBuffers(VulkanContext::getInstance()->getDevice(), &commandBufferAllocateInfo, pCommandBuffer);
    }
}
