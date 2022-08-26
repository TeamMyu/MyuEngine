#pragma once

#include "tiny_obj_loader.h"

#include "../VulkanWrapper/Vulkan.hpp"
#include "../VulkanWrapper/VulkanDevice.hpp"
#include "../VulkanWrapper/Utils.hpp"
#include "Material.hpp"

#include <vector>

namespace Myu
{
    // class that managing verties, indices, uv coordinates 
    class Mesh
    {
    public:
        void init(const VulkanWrapper::VulkanDevice& device, const std::vector<tinyobj::index_t>& sIndices, const tinyobj::attrib_t& attrib);
        
        void destroy(const VulkanWrapper::VulkanDevice& device);
        
        VkBuffer getVertexBuffer() { return mVertexBuffer; }
        VkBuffer getIndexBuffer() {return mIndexBuffer;}
        uint32_t getIndicesSize() {return static_cast<uint32_t>(mIndices.size());}
        Material& getMaterial() {return *material;}
        
        std::shared_ptr<Material> material;

        std::string name;
    private:
        std::vector<VulkanWrapper::Vertex> mVertices;
        std::vector<uint32_t> mIndices;
        
        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;
        
        VkBuffer mIndexBuffer;
        VkDeviceMemory mIndexBufferMemory;
    };
}
