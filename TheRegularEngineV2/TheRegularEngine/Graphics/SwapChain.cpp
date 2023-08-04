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

	void SwapChain::CreateSwapChain(uint32_t* width, uint32_t* height)
	{

	}

	void SwapChain::DeleteSwapChain()
	{
		if (m_WindowSurface)
			vkDestroySurfaceKHR(m_Instance, m_WindowSurface, nullptr);

		if (m_SwapChain)
			vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, nullptr);
	}

	void SwapChain::FindImageFormatAndColorSpace()
	{
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		uint32_t FormatCount;
		if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, nullptr); Result != VK_SUCCESS)
		{
			std::cout << "Surface Format bad result at " << __FILE__ << " and line " << __LINE__ << std::endl;
			assert(FormatCount > 0);
		}

		std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
		if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, nullptr); Result != VK_SUCCESS)
		{
			std::cout << "Unable to get Surface Format at " << __FILE__ << " and line " << __LINE__ << std::endl;
		}

		if (FormatCount == 1)
		{
			if (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			{
				m_SwapChainSettings.m_ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				m_SwapChainSettings.m_SurfaceFormat = VK_FORMAT_R8G8B8A8_UNORM;
			}
		}
		else
		{
			bool FormatFound = false;
			for (const auto& Format : SurfaceFormats)
			{
				if (Format.format == VK_FORMAT_R8G8B8A8_UNORM)
				{
					m_SwapChainSettings.m_ColorSpace = Format.colorSpace;
					m_SwapChainSettings.m_SurfaceFormat = VK_FORMAT_R8G8B8A8_UNORM;
					FormatFound = true;
				}
			}
			
			if (!FormatFound)
			{
				m_SwapChainSettings.m_ColorSpace = SurfaceFormats[0].colorSpace;
				m_SwapChainSettings.m_SurfaceFormat = SurfaceFormats[0].format;
			}
		}
	}
}