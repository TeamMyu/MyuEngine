#pragma once

#include "VulkanUtils.hpp"

#include <algorithm>

namespace VulkanWrapper
{
	struct SwapchainSpecification
	{
		GLFWwindow* window;
		VkSurfaceKHR surface;
	};

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(const SwapchainSpecification& spec);

		~VulkanSwapchain();

		VkSwapchainKHR& GetVkSwapChain() { return m_SwapChain; }
		std::vector<VkImage> GetVkImages() { return m_SwapChainImages; }
		VkFormat GetVkFormat() { return m_SwapChainImageFormat; }
		VkExtent2D GetVkExtent2D() { return m_SwapChainExtent; }

	private:
		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
		VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) const;
	};
}
