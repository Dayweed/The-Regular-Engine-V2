#pragma once
#include "Device.h"

//Forward declaration to prevent include header just for this
struct GLFWwindow;

namespace TRE
{
	#define MAX_FRAMES_IN_FLIGHT 3

	class SwapChain
	{
		public:
			struct SwapChainSettings
			{
				VkColorSpaceKHR m_ColorSpace;
				VkFormat m_SurfaceFormat;
			};

			struct SwapChainImage
			{
				VkImage Image = nullptr;
				VkImageView ImageView = nullptr;
			};

			SwapChain() = default;
			void Initialize(VkInstance Instance, const std::shared_ptr<Device>& LogicalDevice, GLFWwindow* Handle);
			void CreateSwapChain(uint32_t* width, uint32_t* height, bool Vsync);
			void FindImageFormatAndColorSpace();
			void DestroySwapChain();
			
			void BeginFrame();
			void Present();
			uint32_t AccuireNextImage();
			void Resize(uint32_t width, uint32_t height);

		public:
			VkRenderPass GetRenderPass();
			uint32_t GetImageCount();
			uint32_t GetWidth();
			uint32_t GetHeight();
			uint32_t GetQueueIndex();
			VkFramebuffer GetCurrentFrameBuffer();
			VkCommandBuffer GetCurrentCommandBuffer();
			uint32_t GetCurrentBufferIndex();
			VkFormat GetColorFormat();
			VkSemaphore GetRenderComplete();
			VkExtent2D GetSwapChainExtent();
			SwapChainSettings GetSwapChainSettings();
			std::vector<SwapChainImage> GetCurrentSwapChainImage();
			uint32_t GetCurrentImageIndex();

		private:
			VkInstance m_Instance = nullptr;
			std::shared_ptr<Device> m_LogicalDevice;
		
		private:
			VkSwapchainKHR m_SwapChain = nullptr;
			VkSurfaceKHR m_WindowSurface;
			SwapChainSettings m_SwapChainSettings;

			uint32_t m_Width;
			uint32_t m_Height;
			VkExtent2D m_Extent;

			std::vector<SwapChainImage> m_SwapChainImages;
			std::vector<VkImage> m_VulkanImages;
			uint32_t m_ImageCount = 0;

			struct SwapChainCommandBuffer
			{
				VkCommandPool CommandPool;
				VkCommandBuffer CommandBuffer;
			};
			std::vector<SwapChainCommandBuffer> m_CommandBuffers;

			struct
			{
				VkSemaphore PresentComplete = nullptr; //Swap Chain present
				VkSemaphore RenderComplete = nullptr; //Command Buffer
			} m_Semaphores;

			std::vector<VkFence> m_WaitFences;
			VkRenderPass m_Renderpass;

			VkRenderPass m_RenderPass;
			uint32_t m_CurrentBufferIndex = 0;
			uint32_t m_CurrentImageIndex = 0;

			std::vector<VkFramebuffer> m_FrameBuffers;

			uint32_t m_QueueIndex = UINT32_MAX;
	};
}