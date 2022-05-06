#pragma once

#include <vulkan/vulkan.h>

namespace Myu::VulkanWrapper::Utils
{
    inline VkDescriptorBufferInfo createDescBufferInfo(VkBuffer& buffer, VkDeviceSize offset, VkDeviceSize range)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range  = range;
        return bufferInfo;
    }

    inline VkDescriptorImageInfo createDescImageInfo(VkImageView& view, VkSampler& sampler)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = view;
        imageInfo.sampler     = sampler;
        return imageInfo;
    }
}
