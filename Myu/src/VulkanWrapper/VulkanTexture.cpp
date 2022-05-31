#include "VulkanTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#include <vulkan/vulkan.h>

#include "Utils.hpp"


namespace Myu::VulkanWrapper
{
    VulkanTexture::VulkanTexture()
    {
        mSpec.samplerInfo.sType                  = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        mSpec.samplerInfo.magFilter               = VK_FILTER_LINEAR;
        mSpec.samplerInfo.minFilter               = VK_FILTER_LINEAR;
        mSpec.samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        mSpec.samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        mSpec.samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        mSpec.samplerInfo.anisotropyEnable        = VK_TRUE;
        mSpec.samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        mSpec.samplerInfo.unnormalizedCoordinates = VK_FALSE;
        mSpec.samplerInfo.compareEnable           = VK_FALSE;
        mSpec.samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        mSpec.samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        mSpec.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        mSpec.imageLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
 
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

    void VulkanTexture::createTextureTarget(VulkanDevice *device, uint32_t w, uint32_t h, VkFormat format)
    {
        this->mDevice = device;
        this->width   = w;
        this->height  = h;

        createImage(mDevice->GetVkPhysicalDevice(), mDevice->GetVkLogicalDevice(), w, h, format, VK_IMAGE_TILING_OPTIMAL, mSpec.imageUsageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

        Utils::transitionImageLayout(*mDevice, mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, mSpec.imageLayout);
        Utils::createImageView(device->GetVkLogicalDevice(), mImage, &mImageView, format, VK_IMAGE_ASPECT_COLOR_BIT);

        // get maxSamplerAnisotropy for sampler
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device->GetVkPhysicalDevice(), &properties);

        mSpec.samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        VK_CHECK_RESULT(vkCreateSampler(device->GetVkLogicalDevice(), &mSpec.samplerInfo, nullptr, &mSampler));
    }

    void VulkanTexture::loadFromFile(VulkanDevice *device, const char *filePath)
    {
        this->mDevice = device;
        
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        this->width = texWidth;
        this->height = texHeight;

        if (!pixels)
        {
            std::cout << "null image" << std::endl;
            unsigned char aa[15]{0};
            
            stbi_uc nullImage = (stbi_uc)aa;
            pixels            = &nullImage;
            texWidth          = 4;
            texHeight         = 4;
            texChannels       = 1;
            //throw std::runtime_error("failed to load texture image!");
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Utils::createBuffer(*device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
        
        void *data;
        vkMapMemory(mDevice->GetVkLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(mDevice->GetVkLogicalDevice(), stagingBufferMemory);

        if (imageSize != 64 || texChannels != 1) stbi_image_free(pixels);

        createImage(mDevice->GetVkPhysicalDevice(), mDevice->GetVkLogicalDevice(), texWidth, texHeight,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL, mSpec.imageUsageFlags,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    mImage, mImageMemory);

        Utils::transitionImageLayout(*mDevice, mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        Utils::copyBufferToImage(*mDevice, stagingBuffer, mImage, TO_UINT32(texWidth), TO_UINT32(texHeight));
        
        Utils::transitionImageLayout(*mDevice, mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mSpec.imageLayout);

        vkDestroyBuffer(device->GetVkLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device->GetVkLogicalDevice(), stagingBufferMemory, nullptr);
        
        Utils::createImageView(device->GetVkLogicalDevice(), mImage, &mImageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        
        // get maxSamplerAnisotropy for sampler
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device->GetVkPhysicalDevice(), &properties);
        
        mSpec.samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;

        VK_CHECK_RESULT(vkCreateSampler(device->GetVkLogicalDevice(), &mSpec.samplerInfo, nullptr, &mSampler));
    }
}
