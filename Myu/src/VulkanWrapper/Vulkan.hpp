#pragma once

#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define TO_UINT32(x) static_cast<uint32_t>(x)
#define TO_SIZE(x)   static_cast<size_t>(x)

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "../Core/Utils.hpp"

namespace Myu::VulkanWrapper
{
    const int MAX_FRAMES_IN_FLIGHT = 2;

    struct Vertex
    {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::array<VkVertexInputBindingDescription, 1> getBindingDescription()
        {
            std::array<VkVertexInputBindingDescription, 1> bindingDescription{};
            bindingDescription[0].binding   = 0;
            bindingDescription[0].stride    = sizeof(Vertex);
            bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

            attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
            attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
            attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
            attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && normal == other.normal &&
                   uv == other.uv;
        }
    };
    struct PushConstantObject
    {
        glm::vec3 offset;
        alignas(16) glm::vec3 color;
    };
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    // clang-format off
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createPipelineLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout, VkPipelineLayout* pipelineLayout,  int pushConstantSize);
    void createPipelineLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout, VkPipelineLayout* pipelineLayout);

    void createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSets, std::vector<VkBuffer>& uniformBuffers, VkImageView textureImageView, VkSampler textureSampler);

    void copyBufferToImage (VkDevice device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool);

    void updateUniformBuffer(VkDevice device, VkExtent2D swapChainExtent, VkDeviceMemory &uniformBuffersMemory, glm::mat4 modelMat, glm::mat4 viewMat, glm::mat4 projMat);

    void createDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout *descriptorSetLayout);

    VkCommandBuffer beginSingleTimeCommands (VkDevice device, VkCommandPool commandPool);
    void            endSingleTimeCommands   (VkDevice device, VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool);

    void bindPushConstant(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,const void* pushConstant);
    void bindModelBuffer(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer);

    void bindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::vector<VkDescriptorSet> &descriptorSets, int currentFrame);

    void bindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet &descriptorSet);

    void drawIndexedModel(VkCommandBuffer commandBuffer, std::vector<uint32_t> indices);

    void recordCommandBuffer(VkCommandBuffer commandBuffer);
    void endCommandBuffer(VkCommandBuffer commandBuffer);

    VkImageView createImageView (VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void        createImage     (VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createTextureImageView (VkDevice device, VkImage textureImage);

    void createTextureSampler(VkPhysicalDevice physicalDevice, VkDevice device, VkSampler& textureSampler);

    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stageFlags, uint32_t count);

    void createUniformDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet,  VkBuffer &uniformBuffer);

    // clang-format on
}  // namespace Myu::VulkanWrapper

namespace std
{
    template <>
    struct hash<Myu::VulkanWrapper::Vertex>
    {
        size_t operator()(Myu::VulkanWrapper::Vertex const& vertex) const
        {
//            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//                   (hash<glm::vec2>()(vertex.uv) << 1);
            size_t seed = 0;
            Utils::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std
