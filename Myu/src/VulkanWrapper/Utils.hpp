#pragma once

#include "VulkanDevice.hpp"

#define TO_UINT32(x) static_cast<uint32_t>(x)

#define VK_CHECK_RESULT(f)                                                                               \
{                                                                                                        \
    VkResult res = (f);                                                                                  \
    if (res != VK_SUCCESS)                                                                               \
    {                                                                                                    \
        std::cout << "Fatal: VkResult is \"" << Myu::VulkanWrapper::Utils::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        assert(res == VK_SUCCESS);                                                                       \
    }                                                                                                    \
}

namespace Myu::VulkanWrapper::Utils
{
    std::string errorString(VkResult errorCode);

    void transitionImageLayout(const VulkanDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkCommandBuffer beginSingleTimeCommands(const VulkanDevice& device);
    void endSingleTimeCommands(const VulkanDevice& device, VkCommandBuffer commandBuffer);

    void copyBufferToImage(const VulkanDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createImageView(VkDevice device, VkImage image, VkImageView* pView, VkFormat format, VkImageAspectFlags aspectFlags);\

    void createDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet,  VkBuffer &uniformBuffer, VkImageView& imgView, VkSampler& sampler);
}
