#include "pch.h"
#include "SwapChain.h"

namespace TRE
{
	void SwapChain::Initialize(VkInstance Instance, const std::shared_ptr<LogicalDevice>& LogicalDevice)
	{
		m_Instance = Instance;
		m_LogicalDevice = LogicalDevice;


	}

	void SwapChain::InitializeWindowSurface(GLFWwindow* Handle)
	{

	}
}