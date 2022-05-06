#pragma once

#include "VulkanDevice.hpp"
#include <string>

namespace Myu::VulkanWrapper
{
    class VulkanTexture
    {
    public:
        void destroy();
        void loadFromFile(VulkanDevice* device, std::string filePath);
        VkImageView& getImageView() {return mImageView;}
        VkSampler& getSampler() {return mSampler;}
        
    private:
        VulkanDevice *mDevice;
        VkImage mImage;
        VkImageLayout mImageLayout;
        VkDeviceMemory mImageMemory;
        VkDescriptorImageInfo mDescriptor;
        VkSampler mSampler;
        VkImageView mImageView;
    };
}
