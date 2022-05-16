#pragma once

#include "../VulkanWrapper/VulkanDevice.hpp"
#include "../VulkanWrapper/Vulkan.hpp"
#include "../VulkanWrapper/Utils.hpp"
#include "Mesh.hpp"

namespace Myu
{
    class Model
    {
    public:
        Model(VulkanWrapper::VulkanDevice& vulkanDevice, const std::string& MODEL_PATH);
        
        Model(VulkanWrapper::VulkanDevice& vulkanDevice, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices);
        
        ~Model();
        void bind(VkCommandBuffer& commandBuffer, VkPipelineLayout pipelineLayout, glm::mat4 modelMat, glm::mat4 viewMat, glm::mat4 projMat);
        void draw(VkCommandBuffer commandBuffer);
        std::vector<Mesh> getMeshes(){return mMeshes;}

    private:
        void loadModelFromPath(const std::string& MODEL_PATH, std::vector<VulkanWrapper::Vertex> &vertices, std::vector<uint32_t> &indices);
        
        VulkanWrapper::VulkanDevice& m_rVulkanDevice;
        
        std::vector<VulkanWrapper::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        
        std::vector<Mesh> mMeshes;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkDescriptorSet m_descriptorSet;
    };
}
