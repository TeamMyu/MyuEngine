#include "Material.hpp"

namespace Myu
{
    Material::Material(VulkanWrapper::VulkanDevice* device, tinyobj::material_t matInfo)
    {
        // create texture
        //FIXME: temp
        std::string diffuseTexname  = "textures/" + matInfo.diffuse_texname;
        std::string ambientTexname  = "textures/" + matInfo.ambient_texname;
        std::string specularTexname = "textures/" + matInfo.specular_texname;
        std::string normalTexname   = "textures/" + matInfo.normal_texname;

        VulkanWrapper::VulkanTexture diffuseTexture;
        VulkanWrapper::VulkanTexture ambientTexture;
        VulkanWrapper::VulkanTexture specularTexture;
        VulkanWrapper::VulkanTexture normalTexture;

        diffuseTexture.loadFromFile(device, (const char*)diffuseTexname.c_str());
        mTextures.push_back(diffuseTexture);

        ambientTexture.loadFromFile(device, (const char*)ambientTexname.c_str());
        mTextures.push_back(ambientTexture);

        specularTexture.loadFromFile(device, (const char*)specularTexname.c_str());
        mTextures.push_back(specularTexture);

        normalTexture.loadFromFile(device, (const char*)normalTexname.c_str());
        mTextures.push_back(normalTexture);

        // create uniform buffer
        VulkanWrapper::Utils::createUniformBuffer(*device, &mUniformBuffer, &mUniformBufferMemory);

        //FIXME: optimize this
        VulkanWrapper::Utils::DescriptorAllocator   descAllocator;
        VulkanWrapper::Utils::DescriptorLayoutCache descLayoutCache;
        descAllocator.init(device->GetVkLogicalDevice());
        descLayoutCache.init(device->GetVkLogicalDevice());

        auto uniformBufferInfo = VulkanWrapper::Utils::createDescBufferInfo(mUniformBuffer, 0, sizeof(VulkanWrapper::UniformBufferObject));

        auto TextureInfo  = VulkanWrapper::Utils::createDescImageInfo(mTextures[0].getImageView(), mTextures[0].getSampler());
        auto TextureInfo2 = VulkanWrapper::Utils::createDescImageInfo(mTextures[1].getImageView(), mTextures[1].getSampler());
        auto TextureInfo3 = VulkanWrapper::Utils::createDescImageInfo(mTextures[2].getImageView(), mTextures[2].getSampler());
        auto TextureInfo4 = VulkanWrapper::Utils::createDescImageInfo(mTextures[3].getImageView(), mTextures[3].getSampler());

        auto builder = VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                           .bindBuffer(&uniformBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                           .bindImage(&TextureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT)
                           .bindImage(&TextureInfo2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT)
                           .bindImage(&TextureInfo3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT)
                           .bindImage(&TextureInfo4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT);
        for (auto mTexture : mTextures)
        {
              // imageview 는 개별, sampler는 공유 가능
            //builder                                                                                                        // 버텍스 세이더에 바인딩
        }

        builder.build(mDescriptorSet, mDescriptorLayout);
    }

    Material::~Material()
    {
    }

}  // namespace Myu
