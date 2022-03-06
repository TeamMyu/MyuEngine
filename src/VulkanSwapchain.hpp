#pragma once

#include <VulkanWrapper.hpp>

namespace VulkanWrapper
{
	class VulkanSwapchain
	{
	public:
		void createSwapChain(GLFWwindow* window, const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface);

		const VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)	const;
		const VkPresentModeKHR	 chooseSwapPresentMode	(const std::vector<VkPresentModeKHR>&availablePresentModes) const;
		const VkExtent2D		 chooseSwapExtent		(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) const;
		
		/* 캡슐화 불가능 및 변경가능으로 인해 버그발생 여지 있음
		inline const auto& getInstance()	const { return m_SwapChain; }
		inline const auto& getImages()		const { return m_SwapChainImages; }
		inline const auto& getImageFormat() const { return m_SwapChainImageFormat; }
		inline const auto& getExtent()		const { return m_SwapChainExtent; }
		반환값 수정x 참조형으로 전달 -> 받을때 참조형으로 받아야함
		*/
		VkSwapchainKHR			m_SwapChain;
		std::vector<VkImage>	m_SwapChainImages;
		VkFormat				m_SwapChainImageFormat;
		VkExtent2D				m_SwapChainExtent;

	private:

	};
}


