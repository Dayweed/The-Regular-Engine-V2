#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"
#include "Graphics/PhysicalDevice.h"

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
			void BeginFrame();
			void SwapBuffers();

			GLFWwindow* GetWindowHandle() const;
			WindowConfig& GetWindowConfig();
			std::shared_ptr<RendererContext> GetRenderContext();
			std::shared_ptr<SwapChain> GetSwapChain();
			VkSurfaceKHR GetSurface() { return m_WindowSurface; }
			VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
			VkDevice GetDevice() { return m_LogicalDevice; }

			//TBR
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			VkDevice m_LogicalDevice;
			VkQueue m_GraphicsQueue;
			VkQueue m_PresentQueue;
			const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			void CreateLogicalDevice();
			

			std::shared_ptr<PhysicalDevice> m_PhysicalDevice; //Auto destroyed when instance is destroyed

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;
			VkSurfaceKHR m_WindowSurface;
			std::shared_ptr<RendererContext> m_RenderContext;
			std::shared_ptr<SwapChain> m_SwapChain;

			static bool FrameBufferResized;
	};
}