#pragma once

#include "../VulkanWrapper/VulkanDevice.hpp"
#include "../VulkanWrapper/Vulkan.hpp"

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
        
        VkBuffer& getUniformBuffer(){return m_uniformBuffer;}
        VkDeviceMemory& getUniformMemory() {return m_uniformBufferMemory;}
        VkDescriptorSet& getDescriptorSet() {return m_descriptorSet;}

    private:
        void loadModelFromPath(const std::string& MODEL_PATH, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices);
        
        VulkanWrapper::VulkanDevice& m_rVulkanDevice;
        
        std::vector<VulkanWrapper::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        VkBuffer m_uniformBuffer;
        VkDeviceMemory m_uniformBufferMemory;
        VkDescriptorSet m_descriptorSet;

        glm::mat4 m_ModelMatrix;
    };
}
