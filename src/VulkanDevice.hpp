#pragma once

#include "VulkanUtils.hpp"
//#include "VulkanSwapchain.hpp"

#include <vector>
#include <optional>
#include <set>
#include <string>

namespace VulkanWrapper
{
	class VulkanDevice
	{
	public:
		VulkanDevice(const VkSurfaceKHR &surface);
		~VulkanDevice();

		VkPhysicalDevice GetVkPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice GetVkLogicalDevice() { return m_Device; }
		VkQueue GetVkGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetVkPresentQueue() { return m_PresentQueue; }
		QueueFamilyIndices GetQueueFamilyIndices() { return m_indices; }

	private:
		void createPhysicalDevice(const VkSurfaceKHR &surface);
		void createLogicalDevice(const VkSurfaceKHR &surface);

		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		QueueFamilyIndices m_indices;
	};
}
