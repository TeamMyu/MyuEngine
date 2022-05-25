#include "Utils.hpp"

namespace Myu::VulkanWrapper::Utils
{
    VkDescriptorPool createDescPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(poolSizes.sizes.size());
        for (auto sz : poolSizes.sizes) {
            sizes.push_back({ sz.first, uint32_t(sz.second * count) });
        }
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = flags;
        pool_info.maxSets = count;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes = sizes.data();

        VkDescriptorPool descriptorPool;
        vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

        return descriptorPool;
    }

    void DescriptorAllocator::resetPools()
    {
        for (auto p : usedPools)
        {
            vkResetDescriptorPool(device, p, 0);
        }

        freePools = usedPools;
        usedPools.clear();
        currentPool = VK_NULL_HANDLE;
    }

    bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
    {
        if (currentPool == VK_NULL_HANDLE)
        {
            currentPool = getPool();
            usedPools.push_back(currentPool);
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;

        allocInfo.pSetLayouts = &layout;
        allocInfo.descriptorPool = currentPool;
        allocInfo.descriptorSetCount = 1;
        

        VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
        bool needReallocate = false;

        switch (allocResult) {
        case VK_SUCCESS:
            //all good, return
            return true;

            break;
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            //reallocate pool
            needReallocate = true;
            break;
        default:
            //unrecoverable error
            return false;
        }
        
        if (needReallocate)
        {
            //allocate a new pool and retry
            currentPool = getPool();
            usedPools.push_back(currentPool);

            allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);

            //if it still fails then we have big issues
            if (allocResult == VK_SUCCESS)
            {
                return true;
            }
        }

        return false;
    }

    void DescriptorAllocator::init(VkDevice newDevice)
    {
        device = newDevice;
    }

    void DescriptorAllocator::cleanup()
    {
        //delete every pool held
        for (auto p : freePools)
        {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
        for (auto p : usedPools)
        {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
    }

    VkDescriptorPool DescriptorAllocator::getPool()
    {
        if (freePools.size() > 0)
        {
            VkDescriptorPool pool = freePools.back();
            freePools.pop_back();
            return pool;
        }
        else {
            return createDescPool(device, descriptorSizes, 1000, 0);
        }
    }

    void DescriptorLayoutCache::init(VkDevice newDevice)
    {
        device = newDevice;
    }

    VkDescriptorSetLayout DescriptorLayoutCache::createDescLayout(VkDescriptorSetLayoutCreateInfo* info)
    {
        DescriptorLayoutInfo layoutinfo;
        layoutinfo.bindings.reserve(info->bindingCount);
        bool isSorted = true;
        int32_t lastBinding = -1;
        for (uint32_t i = 0; i < info->bindingCount; i++) {
            layoutinfo.bindings.push_back(info->pBindings[i]);

            //check that the bindings are in strict increasing order
            if (static_cast<int32_t>(info->pBindings[i].binding) > lastBinding)
            {
                lastBinding = info->pBindings[i].binding;
            }
            else{
                isSorted = false;
            }
        }
        if (!isSorted)
        {
            std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b ) {
                return a.binding < b.binding;
            });
        }
        
        auto it = layoutCache.find(layoutinfo);
        if (it != layoutCache.end())
        {
            return (*it).second;
        }
        else {
            VkDescriptorSetLayout layout;
            vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

            //layoutCache.emplace()
            //add to cache
            layoutCache[layoutinfo] = layout;
            return layout;
        }
    }


    void DescriptorLayoutCache::cleanup()
    {
        //delete every descriptor layout held
        for (auto pair : layoutCache)
        {
            vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
        }
    }

    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
    {
        DescriptorBuilder builder;
        
        builder.cache = layoutCache;
        builder.alloc = allocator;
        return builder;
    }


    DescriptorBuilder& DescriptorBuilder::bindBuffer(VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        VkDescriptorSetLayoutBinding newBinding{};

        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = bindings.size();

        bindings.push_back(newBinding);

        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;

        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pBufferInfo = bufferInfo;
        newWrite.dstBinding = newBinding.binding;

        writes.push_back(newWrite);
        return *this;
    }


    DescriptorBuilder& DescriptorBuilder::bindImage(VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        VkDescriptorSetLayoutBinding newBinding{};

        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = bindings.size();

        bindings.push_back(newBinding);

        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;

        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pImageInfo = imageInfo;
        newWrite.dstBinding = newBinding.binding;

        writes.push_back(newWrite);
        return *this;
    }

    bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
    {
        //build layout first
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;

        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());

        layout = cache->createDescLayout(&layoutInfo);

        //allocate descriptor
        bool success = alloc->allocate(&set, layout);
        if (!success) { return false; };

        //write descriptor
        for (VkWriteDescriptorSet& w : writes) {
            w.dstSet = set;
        }

        vkUpdateDescriptorSets(alloc->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        return true;
    }


    bool DescriptorBuilder::build(VkDescriptorSet& set)
    {
        VkDescriptorSetLayout layout;
        return build(set, layout);
    }


    bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
    {
        if (other.bindings.size() != bindings.size())
        {
            return false;
        }
        else {
            //compare each of the bindings is the same. Bindings are sorted so they will match
            for (int i = 0; i < bindings.size(); i++) {
                if (other.bindings[i].binding != bindings[i].binding)
                {
                    return false;
                }
                if (other.bindings[i].descriptorType != bindings[i].descriptorType)
                {
                    return false;
                }
                if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
                {
                    return false;
                }
                if (other.bindings[i].stageFlags != bindings[i].stageFlags)
                {
                    return false;
                }
            }
            return true;
        }
    }

    size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
    {
        using std::size_t;
        using std::hash;

        size_t result = hash<size_t>()(bindings.size());

        for (const VkDescriptorSetLayoutBinding& b : bindings)
        {
            //pack the binding data into a single int64. Not fully correct but its ok
            size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

            //shuffle the packed binding data and xor it with the main hash
            result ^= hash<size_t>()(binding_hash);
        }

        return result;
    }

    std::string errorString(VkResult errorCode)
    {
        switch (errorCode)
        {
    #define STR(r) case VK_ ##r: return #r
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
    #undef STR
        default:
            return "UNKNOWN_ERROR";
        }
    }

    VkCommandBuffer beginSingleTimeCommands(const VulkanDevice& device)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = device.GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.GetVkLogicalDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(const VulkanDevice& device, VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(device.GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.GetVkGraphicsQueue());

        vkFreeCommandBuffers(device.GetVkLogicalDevice(), device.GetVkCommandPool(), 1, &commandBuffer);
    }

    void transitionImageLayout(const VulkanDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(device);

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer,
                             sourceStage,
                             destinationStage,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        endSingleTimeCommands(device, commandBuffer);
    }

    void copyBufferToImage(const VulkanDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(device);

        VkBufferImageCopy region{};
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(device, commandBuffer);
    }

    void createImageView(VkDevice device, VkImage image, VkImageView* pView, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = format;
        viewInfo.subresourceRange.aspectMask     = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, pView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void createDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet,  VkBuffer &uniformBuffer, VkImageView& imgView, VkSampler& sampler)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &descriptorSetLayout;
        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imgView;
        imageInfo.sampler = sampler;
        
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void createVertexBuffer(const VulkanDevice& device, const std::vector<Vertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory)
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        VkBuffer       stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *data;
        vkMapMemory(device.GetVkLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device.GetVkLogicalDevice(), stagingBufferMemory);

        createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(device, stagingBuffer, *vertexBuffer, bufferSize);

        vkDestroyBuffer(device.GetVkLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.GetVkLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void createIndexBuffer(const VulkanDevice& device, const std::vector<uint32_t>& indices, VkBuffer *pIndexBuffer, VkDeviceMemory* pIndexBufferMemory)
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer       stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *data;
        vkMapMemory(device.GetVkLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device.GetVkLogicalDevice(), stagingBufferMemory);

        createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pIndexBuffer, pIndexBufferMemory);

        copyBuffer(device, stagingBuffer, *pIndexBuffer, bufferSize);

        vkDestroyBuffer(device.GetVkLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.GetVkLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void createUniformBuffer(const VulkanDevice& device, VkBuffer* pBuffer, VkDeviceMemory* pMemory)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pBuffer, pMemory);
    }

    void createBuffer(const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* pBuffer, VkDeviceMemory* pBufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device.GetVkLogicalDevice(), &bufferInfo, nullptr, pBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.GetVkLogicalDevice(), *pBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(device.GetVkPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.GetVkLogicalDevice(), &allocInfo, nullptr, pBufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device.GetVkLogicalDevice(), *pBuffer, *pBufferMemory, 0);
    }

    void copyBuffer(const VulkanDevice& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(device);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(device, commandBuffer);
    }

    void updateUniformBuffer(VkDevice device, VkDeviceMemory &uniformBuffersMemory, glm::mat4 modelMat, glm::mat4 viewMat, glm::mat4 projMat)
    {
        UniformBufferObject ubo{};
        ubo.model = modelMat;
        ubo.view = viewMat;
        ubo.proj = projMat;

        void *data;
        vkMapMemory(device, uniformBuffersMemory, 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBuffersMemory);
    }

    void bindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet &descriptorSet)
    {
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0,
                                1,
                                &descriptorSet,
                                0,
                                nullptr);
    }
}
