#include "Mesh.hpp"

namespace Myu
{
    void Mesh::init(const VulkanWrapper::VulkanDevice& device, const std::vector<tinyobj::index_t>& sIndices, const tinyobj::attrib_t& attrib)
    {
        mVertices.clear();
        mIndices.clear();
        
        // TODO: Vertices between meshes need to be optimized
        std::unordered_map<VulkanWrapper::Vertex, uint32_t> uniqueVertices{};
        for (const auto &index : sIndices)
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
                vertex.uv.y = 1 - vertex.uv.y;
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
                mVertices.push_back(vertex);
            }
            mIndices.push_back(uniqueVertices[vertex]);
        }
        
        VulkanWrapper::Utils::createVertexBuffer(device, mVertices, &mVertexBuffer, &mVertexBufferMemory);
        VulkanWrapper::Utils::createIndexBuffer(device, mIndices, &mIndexBuffer, &mIndexBufferMemory);
    }

    void Mesh::destroy(const VulkanWrapper::VulkanDevice &device)
    {
        vkDestroyBuffer(device.GetVkLogicalDevice(), mVertexBuffer, nullptr);
        vkFreeMemory(device.GetVkLogicalDevice(), mVertexBufferMemory, nullptr);
        
        vkDestroyBuffer(device.GetVkLogicalDevice(), mIndexBuffer, nullptr);
        vkFreeMemory(device.GetVkLogicalDevice(), mIndexBufferMemory, nullptr);

    }
}
