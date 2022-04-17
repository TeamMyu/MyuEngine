#pragma once

#include "../VulkanWrapper/VulkanDevice.hpp"

namespace Myu
{
    class Model
    {
    public:
        Model(VulkanWrapper::VulkanDevice& vulkanDevice, const std::string& MODEL_PATH);
        
        Model(VulkanWrapper::VulkanDevice& vulkanDevice, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices);
        
        ~Model();
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void loadModelFromPath(const std::string& MODEL_PATH, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices);
        
        VulkanWrapper::VulkanDevice& m_rVulkanDevice;
        
        std::vector<VulkanWrapper::Vertex> vertices;
        std::vector<uint32_t> indices;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;

        glm::mat4 m_ModelMatrix;
    };
}
