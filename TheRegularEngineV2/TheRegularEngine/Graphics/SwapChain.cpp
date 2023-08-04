#include "pch.h"
#include "TREIncludes.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	void SwapChain::Initialize(VkInstance Instance, const std::shared_ptr<LogicalDevice>& LogicalDevice, GLFWwindow* Handle)
	{
		m_Instance = Instance;
		m_LogicalDevice = LogicalDevice;
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		glfwCreateWindowSurface(m_Instance, Handle, nullptr, &m_WindowSurface);

		uint32_t NumberofQueues; 
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, nullptr);
		assert(NumberofQueues > 0);

		std::vector<VkQueueFamilyProperties> QueueProperties(NumberofQueues);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, QueueProperties.data());

		std::vector<VkBool32> SupportsPresent(NumberofQueues);
		for (int x = 0; x < NumberofQueues; x++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, x, m_WindowSurface, &SupportsPresent[x]);
		}

		uint32_t PresentQueue = UINT32_MAX;

		for (int x = 0; x < NumberofQueues; x++)
		{
			if ((QueueProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (SupportsPresent[x] == VK_TRUE)
				{
					m_GraphicsQueueIndex = x;
					PresentQueue = x;
					break;
				}

				if (m_GraphicsQueueIndex == UINT32_MAX)
					m_GraphicsQueueIndex = x;
			}
		}

		if (PresentQueue == UINT32_MAX)
		{
			for (int x = 0; x < NumberofQueues; x++)
			{
				if (SupportsPresent[x] == VK_TRUE)
				{
					PresentQueue = x;
					break;
				}
			}
		}

		assert(PresentQueue != UINT32_MAX);
		assert(m_GraphicsQueueIndex != UINT32_MAX);

		FindImageFormatAndColorSpace();
	}

	void SwapChain::FindImageFormatAndColorSpace()
	{

	}
}