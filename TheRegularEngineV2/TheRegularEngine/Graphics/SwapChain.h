#pragma once
#include "pch.h"
#include "Device.h"
#include "RenderPass.h"
#include "RendererContext.h"

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
				VkFormat m_DepthFormat;
			};

			struct SwapChainImage
			{
				VkImage Image = nullptr;
				VkImageView ImageView = nullptr;
			};

			SwapChain(std::shared_ptr<Device>& LogicalDevice, std::shared_ptr<PhysicalDevice>& PD, GLFWwindow* Handle);
			~SwapChain() = default;
			void Initialize(uint32_t Width, uint32_t Height);
			void BeginFrame();
			void Present();
			void DestroySwapChain();

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
			VkFormat GetDepthFormat();
			VkSemaphore GetRenderComplete();
			VkExtent2D GetSwapChainExtent();
			SwapChainSettings GetSwapChainSettings();
			uint32_t GetCurrentImageIndex();

		public:
			void CreateSwapChain(uint32_t Width, uint32_t Height);
			void RecreateSwapChain();
			void CleanSwapChain();
			void CreateImageViews();
			void CreateRenderPass();
			void CreateFrameBuffer();
			void CreateCommandPool();
			void CreateCommandbuffer();
			void CreateSyncObjects();
			
		private:
			std::shared_ptr<Device> m_LogicalDevice;
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
		
		private:
			VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
			VkSurfaceKHR m_WindowSurface;
			SwapChainSettings m_SwapChainSettings;

			uint32_t m_Width = 1600;
			uint32_t m_Height = 900;
			VkExtent2D m_Extent;

			std::vector<SwapChainImage> m_SwapChainImages;
			std::vector<VkImage> m_VulkanImages;
			uint32_t m_ImageCount = 0;

			VkCommandPool m_CommandPool;
			std::vector<VkCommandBuffer> m_Commandbuffers;

			struct Semaphores
			{
				VkSemaphore PresentComplete = nullptr; //Swap Chain present
				VkSemaphore RenderComplete = nullptr; //Command Buffer
			};

			std::vector<Semaphores> m_Semaphores;
			std::vector<VkFence> m_WaitFences;

			uint32_t m_CurrentBufferIndex = 0;
			uint32_t m_CurrentImageIndex = 0;

			VkRenderPass m_RenderPass;
			std::vector<VkFramebuffer> m_SwapChainFramebuffers;

			uint32_t m_QueueIndex = UINT32_MAX;
	};
}