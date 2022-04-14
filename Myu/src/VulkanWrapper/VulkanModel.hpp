#pragma once

#include "VulkanDevice.hpp"

namespace VulkanWrapper
{
    class VulkanModel
    {
    public:
        VulkanModel(VulkanDevice& vulkanDevice, const std::string& MODEL_PATH);
        ~VulkanModel();
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void loadModel(const std::string& MODEL_PATH, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

    private:
        VulkanDevice& m_rVulkanDevice;

        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        VkBuffer              vertexBuffer;
        VkDeviceMemory        vertexBufferMemory;
        VkBuffer              indexBuffer;
        VkDeviceMemory        indexBufferMemory;

        glm::mat4 m_ModelMatrix;
    };
}
