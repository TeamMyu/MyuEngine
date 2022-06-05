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
 
        auto builder = VulkanWrapper::Utils::DescriptorBuilder::begin(&descLayoutCache, &descAllocator)
                           .bindBuffer(&uniformBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                           .build(mDescriptorSet, mDescriptorLayout);
    }

    Material::~Material()
    {
    }

}  // namespace Myu
