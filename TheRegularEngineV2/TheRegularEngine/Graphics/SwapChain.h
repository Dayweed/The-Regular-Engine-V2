#pragma once
#include "LogicalDevice.h"

//Forward declaration to prevent include header just for this
struct GLFWwindow;

namespace TRE
{
	class SwapChain
	{
		public:
			SwapChain() = default;
			void Initialize(VkInstance Instance, const std::shared_ptr<LogicalDevice>& LogicalDevice, GLFWwindow* Handle);
			void FindImageFormatAndColorSpace();

		private:
			VkInstance m_Instance = nullptr;
			std::shared_ptr<LogicalDevice> m_LogicalDevice;
		
		private:
			VkSwapchainKHR m_SwapChain = nullptr;
			VkSurfaceKHR m_WindowSurface;

			int m_Width;
			int m_Height;

			uint32_t m_GraphicsQueueIndex = UINT32_MAX;
	};
}