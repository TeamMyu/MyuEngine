#include "Material.hpp"

namespace Myu
{
    Material::Material(VulkanWrapper::VulkanDevice* device, tinyobj::material_t matInfo)
    {
        // create texture
        //FIXME: temp
        std::string diffuseTexname = "textures/" + matInfo.diffuse_texname;
        std::string ambientTexname = "textures/" + matInfo.ambient_texname;
        std::string specularTexname = "textures/" + matInfo.specular_texname;
        std::string normalTexname = "textures/" + matInfo.normal_texname;

        VulkanWrapper::VulkanTexture diffuseTexture;
        VulkanWrapper::VulkanTexture ambientTexture;
        VulkanWrapper::VulkanTexture specularTexture;
        VulkanWrapper::VulkanTexture normalTexture;

        if (!matInfo.diffuse_texname.empty())
        {
            diffuseTexture.loadFromFile(device, (const char*)diffuseTexname.c_str());
            mTextures.push_back(diffuseTexture);
        }

        if (!matInfo.ambient_texname.empty())
        {
            ambientTexture.loadFromFile(device, (const char*)ambientTexname.c_str());
            mTextures.push_back(ambientTexture);
        }

        if (!matInfo.specular_texname.empty())
        {
            specularTexture.loadFromFile(device, (const char*)specularTexname.c_str());
            mTextures.push_back(specularTexture);
        }

        if (!matInfo.normal_texname.empty())
        {
            normalTexture.loadFromFile(device, (const char*)normalTexname.c_str());
            mTextures.push_back(normalTexture);
        }

        // create uniform buffer
        VulkanWrapper::Utils::createUniformBuffer(*device, &mUniformBuffer, &mUniformBufferMemory);
        
        //FIXME: optimize this
        VulkanWrapper::Utils::DescriptorAllocator descAllocator;
        VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
        descAllocator.init(device->GetVkLogicalDevice());
        descLayoutCache.init(device->GetVkLogicalDevice());
        
        auto uniformBufferInfo = VulkanWrapper::Utils::createDescBufferInfo(mUniformBuffer, 0, sizeof(VulkanWrapper::UniformBufferObject));

        auto builder = VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                       .bindBuffer(&uniformBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
        for (VulkanWrapper::VulkanTexture vkwTexture : mTextures)
        {
            std::cout << "mat bindings :" << std::endl;
            auto TextureInfo = VulkanWrapper::Utils::createDescImageInfo(vkwTexture.getImageView(), vkwTexture.getSampler());  // imageview 는 개별, sampler는 공유 가능
            builder = builder.bindImage(&TextureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT);     // 버텍스 세이더에 바인딩
        }

        builder.build(mDescriptorSet, mDescriptorLayout);
    }

    Material::~Material()
    {
    }

}
