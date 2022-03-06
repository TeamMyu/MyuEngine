#pragma once

#include <VulkanWrapper.hpp>

namespace VulkanWrapper
{
	class VulkanImageView
	{
	public:
		void VulkanImageView::createImageViews(const VkDevice& device, const std::vector<VkImage>& images, const VkFormat& format, const VkExtent2D& extent);
		std::vector<VkImageView> m_ImageViews;

	private:
		
	};
}

