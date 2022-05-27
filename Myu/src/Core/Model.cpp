#include "Model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Myu
{
    Model::Model(VulkanWrapper::VulkanDevice &vulkanDevice, const std::string& MODEL_PATH)
        : m_rVulkanDevice{vulkanDevice}
    {
        loadModelFromPath(MODEL_PATH, m_vertices, m_indices);
    }

    Model::Model(VulkanWrapper::VulkanDevice& vulkanDevice, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices)
        : m_rVulkanDevice{vulkanDevice}
    {
//        VulkanWrapper::createVertexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), vertices, &vertexBuffer, &vertexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
//
//        VulkanWrapper::createIndexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), indices, &indexBuffer, indexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
//
//        VulkanWrapper::createUniformBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), m_uniformBuffer, m_uniformBufferMemory);
    }

    Model::~Model()
    {
        for (auto& mesh : mMeshes)
        {
            mesh.destroy(m_rVulkanDevice);
        }
    }

    void Model::bind(VkCommandBuffer& commandBuffer, VkPipelineLayout pipelineLayout, Myu::VulkanWrapper::UniformBufferObject ubo)
    {
        for (auto& mesh : mMeshes)
        {
            VkBuffer vertexBuffers[] = {mesh.getVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, mesh.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            
            if (mesh.material != nullptr){
                VulkanWrapper::Utils::updateUniformBuffer(m_rVulkanDevice.GetVkLogicalDevice(), mesh.getMaterial().getUniformBufferMemory(), ubo);
                VulkanWrapper::Utils::bindDescriptorSet(commandBuffer, pipelineLayout, mesh.getMaterial().getDescriptorSet());
            }
            
            vkCmdDrawIndexed(commandBuffer, mesh.getIndicesSize(), 1, 0, 0, 0);
        }
    }

    void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Myu::VulkanWrapper::UniformBufferObject ubo)
    {
    }

    void Model::loadModelFromPath(const std::string& MODEL_PATH, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices)
    {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn, err;
        
        std::string baseDir = "models/";
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), baseDir.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();
        
        int counter = 0;
        std::cout << "shapes size: " << shapes.size() << std::endl;
        std::cout << "material size: " << materials.size() << std::endl;
        for (const auto &shape : shapes)
        {
            auto mesh = Mesh();
            
            auto mat = std::make_shared<Material>(&m_rVulkanDevice, materials[counter++]);
            mesh.material = mat;
            
            mesh.init(m_rVulkanDevice, shape.mesh.indices, attrib);
            mMeshes.push_back(mesh);
        }
    }
}
