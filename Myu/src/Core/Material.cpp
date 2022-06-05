#include "Material.hpp"

namespace Myu
{
    Material::Material(VulkanWrapper::VulkanDevice* device, tinyobj::material_t matInfo)
    {
        // create texture
        //FIXME: temp
        std::string textureName = matInfo.diffuse_texname;
        if (!textureName.empty())
            mTexture.loadFromFile(device, "textures/" + textureName);
        
        // create uniform buffer
        VulkanWrapper::Utils::createUniformBuffer(*device, &mUniformBuffer, &mUniformBufferMemory);
        
        //FIXME: optimize this
        VulkanWrapper::Utils::DescriptorAllocator descAllocator;
        VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
        descAllocator.init(device->GetVkLogicalDevice());
        descLayoutCache.init(device->GetVkLogicalDevice());
        
        auto uniformBufferInfo = VulkanWrapper::Init::createDescBufferInfo(mUniformBuffer, 0, sizeof(VulkanWrapper::UniformBufferObject));
        auto textureInfo = VulkanWrapper::Init::createDescImageInfo(mTexture.getImageView(), mTexture.getSampler());
        
        VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
            .bindBuffer(0, &uniformBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .bindImage(1, &textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(mDescriptorSet, mDescriptorLayout);
    }

    Material::~Material()
    {
    }

}
