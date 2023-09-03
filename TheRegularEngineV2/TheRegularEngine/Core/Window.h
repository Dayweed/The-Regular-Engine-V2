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

	struct QueueFamilies
	{
		int32_t Graphics = -1;
		int32_t Present = -1;

		bool IsComplete() { return ((Graphics != -1) && (Present != -1)); }
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
			QueueFamilies FindQueueFamilies(VkPhysicalDevice dev);
			void PhysicalDeviceSetup();
			uint32_t GetPhysicalDeviceCount();
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
			void CreateLogicalDevice();
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
			SwapChainDetails QuerySwapChainSupprt(VkPhysicalDevice device);

			VkSurfaceFormatKHR ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
			VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);
			void CreateSwapChain();
			void RecreateSwapChain();
			void CleanSwapChain();
			void CreateImageViews();
			void CreateRenderPass();
			void CreateFrameBuffer();
			void CreateCommandPool();
			void CreateCommandbuffer();
			void RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t imageindex);
			void CreateSyncObjects();

			VkSwapchainKHR m_SwapChainee = VK_NULL_HANDLE;
			VkExtent2D m_Extent;
			VkFormat m_Format;
			VkRenderPass m_RenderPass;
			std::vector<VkImage> m_Images;
			std::vector<VkImageView> m_SwapChainImageViews;
			std::vector<VkFramebuffer> m_SwapChainFramebuffers;
			VkCommandPool m_CommandPool;
			std::vector<VkCommandBuffer> m_Commandbuffer;
			std::vector<VkSemaphore> m_ImageAvailable;
			std::vector<VkSemaphore> m_ImageRendered;
			std::vector<VkFence> m_FlightFence;
			uint32_t m_CurrentFrame = 0;

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;

			std::shared_ptr<RendererContext> m_RenderContext;
			std::shared_ptr<SwapChain> m_SwapChain;
			VkSurfaceKHR m_Surface;

			static bool FrameBufferResized;
	};
}