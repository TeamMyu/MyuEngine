#pragma once

#include "Vulkan.hpp"

#include "VulkanSwapchain.hpp"

#include <vector>
#include <optional>
#include <set>

namespace VulkanWrapper
{
	const std::vector<const char *> &m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // ������ġ���� �����ϴ� ť�йи��� ���� ������

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // ť �йи� ������ ������

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{								// ���� �׷��� ����� �����Ѵٸ�
				indices.graphicsFamily = i; // �ε��� ����
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport); // Surface�� ���������̼� ��带 �����ϴ��� ������

			if (presentSupport)
			{
				indices.presentFamily = i; // �����Ѵٸ� �ε��� ����
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices; // �׷��� ť�� ���������̼� ť�� �ٸ� �� ������ ��ȿ���� �Դϴ�.
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr); // ������ġ���� �����ϴ� Ȯ���� ���� ������

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()); // Ȯ����� ���Ϳ� ����

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end()); // �䱸�ϴ� Ȯ����� ������

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName); // ��ġ�ϴ� Ȯ����� ����
		}

		return requiredExtensions.empty(); // ���� ���� ����ִٸ� ��� �����Ѵٴ� ��
	}

	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices = findQueueFamilies(device, surface);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	class VulkanDevice
	{
	public:
		VulkanDevice(const VkSurfaceKHR &surface);
		~VulkanDevice();

		VkPhysicalDevice GetVkPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice GetVkLogicalDevice() { return m_Device; }
		VkQueue GetVkGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetVkPresentQueue() { return m_PresentQueue; }

	private:
		void createPhysicalDevice(const VkSurfaceKHR &surface);
		void createLogicalDevice(const VkSurfaceKHR &surface);

		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
	};
}
