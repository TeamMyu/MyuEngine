#pragma once

#include "VulkanDevice.hpp"
#include <string>

namespace Myu::VulkanWrapper
{
    struct VulkanTextureSpec
    {
        VkSamplerCreateInfo samplerInfo{};
        VkImageUsageFlags   imageUsageFlags;
        VkImageLayout       imageLayout;
    };
    class VulkanTexture
    {
    public:
        VulkanTexture();

        void destroy();
        void loadFromFile(VulkanDevice* device, const char* filePath);
        void createTextureTarget(VulkanDevice* device, uint32_t width, uint32_t height, VkFormat format);
        void createTextureTarget(VulkanDevice* device, uint32_t w, uint32_t h, VkFormat format, VkImageAspectFlagBits flag);

        VkImageView& getImageView() { return mImageView; }
        VkSampler&   getSampler() { return mSampler; }
        VkImage&     getImage() { return mImage; }

        VulkanTextureSpec mSpec;
        uint32_t          width;
        uint32_t          height;

        VkImage               mImage{VK_NULL_HANDLE};
        VkImageLayout         mImageLayout;
        VkDeviceMemory        mImageMemory;
        VkDescriptorImageInfo mDescriptor;
        VkSampler             mSampler;
        VkImageView           mImageView;
        VkFormat              format;

    private:
        VulkanDevice* mDevice;
    };
}  // namespace Myu::VulkanWrapper
