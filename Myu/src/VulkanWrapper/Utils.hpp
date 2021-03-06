#pragma once

#include "VulkanDevice.hpp"

#define TO_UINT32(x) static_cast<uint32_t>(x)

#define VK_CHECK_RESULT(f)                                                                               \
{                                                                                                        \
    VkResult res = (f);                                                                                  \
    if (res != VK_SUCCESS)                                                                               \
    {                                                                                                    \
        std::cout << "Fatal: VkResult is \"" << Myu::VulkanWrapper::Utils::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        assert(res == VK_SUCCESS);                                                                       \
    }                                                                                                    \
}

namespace Myu::VulkanWrapper::Utils
{
    class DescriptorAllocator
    {
    public:
        struct PoolSizes
        {
            std::vector<std::pair<VkDescriptorType,float>> sizes =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
            };
        };

        void resetPools();
        bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);
        
        void init(VkDevice newDevice);

        void cleanup();

        VkDevice device;
    private:
        VkDescriptorPool getPool();

        VkDescriptorPool currentPool{VK_NULL_HANDLE};
        PoolSizes descriptorSizes;
        std::vector<VkDescriptorPool> usedPools;
        std::vector<VkDescriptorPool> freePools;
    };


    class DescriptorLayoutCache
    {
    public:
        void init(VkDevice newDevice);
        void cleanup();

        VkDescriptorSetLayout createDescLayout(VkDescriptorSetLayoutCreateInfo* info);

        struct DescriptorLayoutInfo {
            //good idea to turn this into a inlined array
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            bool operator==(const DescriptorLayoutInfo& other) const;

            size_t hash() const;
        };

    private:
        struct DescriptorLayoutHash
        {

            std::size_t operator()(const DescriptorLayoutInfo& k) const
            {
                return k.hash();
            }
        };

        std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
        VkDevice device;
    };


    class DescriptorBuilder
    {
    public:
        static DescriptorBuilder begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator );

        DescriptorBuilder& bindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        DescriptorBuilder& bindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
        bool build(VkDescriptorSet& set);
    private:
        
        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        

        DescriptorLayoutCache* cache;
        DescriptorAllocator* alloc;
    };

    VkDescriptorPool createDescPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags);

    std::string errorString(VkResult errorCode);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBufferToImage(const VulkanDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createImageView(VkImage image, VkImageView* pView, VkFormat format, VkImageAspectFlags aspectFlags);

    void createDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet,  VkBuffer &uniformBuffer, VkImageView& imgView, VkSampler& sampler);

    void createVertexBuffer(const VulkanDevice& device, const std::vector<Vertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);

    void createIndexBuffer(const VulkanDevice& device, const std::vector<uint32_t>& indices, VkBuffer *pIndexBuffer, VkDeviceMemory* pIndexBufferMemory);

    void createUniformBuffer(const VulkanDevice& device, VkBuffer* pBuffer, VkDeviceMemory* pMemory);

    void createBuffer(const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* pBuffer, VkDeviceMemory* pBufferMemory);

    void copyBuffer(const VulkanDevice& device,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void updateUniformBuffer(VkDevice device, VkDeviceMemory &uniformBuffersMemory, glm::mat4 modelMat, glm::mat4 viewMat, glm::mat4 projMat);

    void bindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet &descriptorSet);

    void copyImage(const VkImage& srcImage, VkImageLayout srcLayout, VkImage& dstImage, VkImageLayout dstLayout);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* pImage, VkDeviceMemory* pImageMemory);
}
