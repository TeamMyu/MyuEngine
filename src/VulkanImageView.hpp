#pragma once

#include "Vulkan.hpp"
#include "VulkanDevice.hpp"

#include <vector>

namespace VulkanWrapper
{
	struct ImageViewSpecification
	{
		std::vector<VkImage> images;
		VkFormat format;
		VkExtent2D extent;
	};

	class VulkanImageView
	{
	public:
		VulkanImageView(const ImageViewSpecification& imageViewSpecification);
		~VulkanImageView();
		std::vector<VkImageView> GetImageViews() { return m_ImageViews; }

	private:
		std::vector<VkImageView> m_ImageView;

	};
}

