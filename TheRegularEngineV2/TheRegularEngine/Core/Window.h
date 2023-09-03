#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"

namespace TRE
{
	struct WindowConfig
	{
		std::string Title = "The Regular Engine";
		uint32_t width = 1600;
		uint32_t height = 900;
		bool Vsync = true;
		bool resize = false;
	};

	class Window
	{
		public:
			Window(const WindowConfig& config = WindowConfig());
			~Window();

			//RAII
			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			void PollEvents();
			int ShouldWindowClose();
			void SwapBuffers();

			GLFWwindow* GetWindowHandle() const;
			WindowConfig& GetWindowConfig();
			std::shared_ptr<RendererContext> GetRenderContext();
			std::shared_ptr<SwapChain> GetSwapChain();
			VkSurfaceKHR GetSurface() { return m_Surface; }
			VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
			VkDevice GetDevice() { return m_LogicalDevice; }
			VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }

			//TBR
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			VkPhysicalDevice m_PhysicalDevice; //Auto destroyed when instance is destroyed
			VkDevice m_LogicalDevice;
			VkQueue m_GraphicsQueue;
			VkQueue m_PresentQueue;
			const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			void PhysicalDeviceSetup();
			uint32_t GetPhysicalDeviceCount();
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
			void CreateLogicalDevice();
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);


		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;

			std::shared_ptr<RendererContext> m_RenderContext;
			std::shared_ptr<SwapChain> m_SwapChain;
			VkSurfaceKHR m_Surface;

			static bool FrameBufferResized;
	};
}