#pragma once

#include <VulkanWrapper.hpp>

namespace VulkanWrapper
{
	class VulkanDevice
	{
	public:
		void createPhysicalDevice	(const VkInstance&		 Instance,		 const VkSurfaceKHR& surface);
		void createLogicalDevice	(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

		/* 캡슐화 불가능 및 변경가능으로 인해 버그발생 여지 있음
		inline const auto& getphysicalDevice() const { return m_PhysicalDevice; }
		inline const auto& getLogicalDevice()  const { return m_Device; }
		inline const auto& getGraphicsQueue()  const { return m_GraphicsQueue; }
		inline const auto& getPresentQueue()   const { return m_PresentQueue; }
		반환값 수정x 참조형으로 전달 -> 받을때 참조형으로 받아야함
		*/
		VkPhysicalDevice m_PhysicalDevice { VK_NULL_HANDLE };
		VkDevice		 m_Device		  { VK_NULL_HANDLE };
		VkQueue			 m_GraphicsQueue  { VK_NULL_HANDLE };
		VkQueue			 m_PresentQueue	  { VK_NULL_HANDLE };

	private:
	};
}


