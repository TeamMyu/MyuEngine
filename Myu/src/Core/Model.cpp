#include "Model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Myu
{
    Model::Model(VulkanWrapper::VulkanDevice &vulkanDevice, const std::string& MODEL_PATH)
        : m_rVulkanDevice{vulkanDevice}
    {
        loadModelFromPath(MODEL_PATH, m_vertices, m_indices);
        
        // FIXME: Duplicated code
        VulkanWrapper::createVertexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), m_vertices, &vertexBuffer, &vertexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
        VulkanWrapper::createIndexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), m_indices, &indexBuffer, indexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
        
        // FIXME: Duplicated code
        VulkanWrapper::createUniformBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), m_uniformBuffer, m_uniformBufferMemory);
    }

    Model::Model(VulkanWrapper::VulkanDevice& vulkanDevice, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices)
        : m_rVulkanDevice{vulkanDevice}
    {
        VulkanWrapper::createVertexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), vertices, &vertexBuffer, &vertexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
        
        VulkanWrapper::createIndexBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), indices, &indexBuffer, indexBufferMemory, vulkanDevice.GetVkGraphicsQueue(), vulkanDevice.GetVkCommandPool());
        
        VulkanWrapper::createUniformBuffer(vulkanDevice.GetVkPhysicalDevice(), vulkanDevice.GetVkLogicalDevice(), m_uniformBuffer, m_uniformBufferMemory);
    }

    Model::~Model()
    {
        vkDestroyBuffer(m_rVulkanDevice.GetVkLogicalDevice(), indexBuffer, nullptr);
        vkFreeMemory(m_rVulkanDevice.GetVkLogicalDevice(), indexBufferMemory, nullptr);

        vkDestroyBuffer(m_rVulkanDevice.GetVkLogicalDevice(), vertexBuffer, nullptr);
        vkFreeMemory(m_rVulkanDevice.GetVkLogicalDevice(), vertexBufferMemory, nullptr);
    }

    void Model::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void Model::draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
    }

    void Model::loadModelFromPath(const std::string& MODEL_PATH, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices)
    {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<VulkanWrapper::Vertex, uint32_t> uniqueVertices{};
        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                VulkanWrapper::Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}
