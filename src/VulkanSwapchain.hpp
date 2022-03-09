#pragma once

#include "Vulkan.hpp"
#include "VulkanDevice.hpp"

#include <GLFW/glfw3.h>
#include <vector>

namespace VulkanWrapper
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;		// �̹��� �ػ�, ť ������ ��
		std::vector<VkSurfaceFormatKHR> formats;	// �ȼ� ����(RGBA ��), �÷� �����̽�(32bpp ��)
		std::vector<VkPresentModeKHR> presentModes; // ȭ�� ��ȯ ���
	};

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); // ���� �Լ���. ���� ����ϰ� ������

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{ // presentMode : ����ü�� ���� �̹����� ȭ��� �����ϴ� ��� ( ���߹��۸�, ��þ��� �� )
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(GLFWwindow* window, const VkSurfaceKHR& surface);

		~VulkanSwapchain();

	private:
		VkSwapchainKHR			m_SwapChain;
		std::vector<VkImage>	m_SwapChainImages;
		VkFormat				m_SwapChainImageFormat;
		VkExtent2D				m_SwapChainExtent;

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
		VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) const;

	};
}


