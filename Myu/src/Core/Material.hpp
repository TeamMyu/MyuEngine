#pragma once

#include "../VulkanWrapper/VulkanDevice.hpp"
#include "../VulkanWrapper/VulkanTexture.hpp"
#include "../VulkanWrapper/Utils.hpp"
#include "../VulkanWrapper/VulkanInitializer.hpp"
#include "tiny_obj_loader.h"

#include <string>

namespace Myu
{
    class Material
    {
    public:
        Material(VulkanWrapper::VulkanDevice* device, tinyobj::material_t matInfo);
        ~Material();
        
        VkDescriptorSet& getDescriptorSet() {return mDescriptorSet;}
        VkDescriptorSetLayout& getDescriptorLayout(){return mDescriptorLayout;}
        VkBuffer getUniformBuffer() {return mUniformBuffer;}
        VkDeviceMemory& getUniformBufferMemory() {return mUniformBufferMemory;}

    private:
        std::vector<VulkanWrapper::VulkanTexture> mTextures;
        VkDescriptorSet mDescriptorSet;
        VkDescriptorSetLayout mDescriptorLayout;
        
        VkBuffer mUniformBuffer;
        VkDeviceMemory mUniformBufferMemory;
    };
}
