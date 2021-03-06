#include "VulkanTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vulkan/vulkan.h>

#include "Utils.hpp"


namespace Myu::VulkanWrapper
{
    void VulkanTexture::destroy()
    {
        vkDestroyImageView(mDevice->GetVkLogicalDevice(), mImageView, nullptr);
        vkDestroyImage(mDevice->GetVkLogicalDevice(), mImage, nullptr);
        if (mSampler)
        {
            vkDestroySampler(mDevice->GetVkLogicalDevice(), mSampler, nullptr);
        }
        vkFreeMemory(mDevice->GetVkLogicalDevice(), mImageMemory, nullptr);
    }

    void VulkanTexture::loadFromFile(VulkanDevice* device, std::string filePath)
    {
        this->mDevice = device;
        
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Utils::createBuffer(*device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
        
        void *data;
        vkMapMemory(mDevice->GetVkLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(mDevice->GetVkLogicalDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        createImage(mDevice->GetVkPhysicalDevice(), mDevice->GetVkLogicalDevice(), texWidth, texHeight,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    mImage, mImageMemory);

        Utils::transitionImageLayout( mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        Utils::copyBufferToImage(*mDevice, stagingBuffer, mImage, TO_UINT32(texWidth), TO_UINT32(texHeight));
        
        Utils::transitionImageLayout(mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device->GetVkLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device->GetVkLogicalDevice(), stagingBufferMemory, nullptr);
        
        Utils::createImageView(mImage, &mImageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        
        // get maxSamplerAnisotropy for sampler
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device->GetVkPhysicalDevice(), &properties);
        
        // create sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VK_CHECK_RESULT(vkCreateSampler(device->GetVkLogicalDevice(), &samplerInfo, nullptr, &mSampler));
    }
}
