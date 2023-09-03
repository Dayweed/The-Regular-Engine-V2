#pragma once
#include "pch.h"
#include "Device.h"
#include "RenderPass.h"
#include "RendererContext.h"
#include "PhysicalDevice.h"

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
				//VkImage DepthImage = nullptr;
				//VkImageView DepthImageView = nullptr;
			};

			SwapChain();
			void Initialize(VkDevice LogicalDevice, GLFWwindow* Handle, VkQueue GraphicsQ, std::shared_ptr<PhysicalDevice>& PD, VkSurfaceKHR Surface);
			void CreateSwapChain(uint32_t* width, uint32_t* height, bool Vsync);
			void FindImageFormatAndColorSpace();
			void DestroySwapChain();
			
			void BeginFrame();
			void Present();

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
			VkSurfaceKHR& GetSurface() { return m_WindowSurface; }

		public: //TBR
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
			SwapChainDetails QuerySwapChainSupprt(VkPhysicalDevice device);
			QueueFamilies FindQueueFamilies(VkPhysicalDevice dev);
			VkSurfaceFormatKHR ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
			VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);
			
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			GLFWwindow* m_Handle;
			VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
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
			VkInstance m_Instance = nullptr;
			VkDevice  m_LogicalDevice;
		
		private:
			VkSwapchainKHR m_SwapChain;
			VkSurfaceKHR m_WindowSurface;
			SwapChainSettings m_SwapChainSettings;

			uint32_t m_Width = 1600;
			uint32_t m_Height = 900;
			//VkExtent2D m_Extent;

			//std::vector<SwapChainImage> m_SwapChainImages;
			std::vector<VkImage> m_SwapChainImages;
			std::vector<VkImageView> m_SwapChainImagesview;
			std::vector<VkImage> m_VulkanImages;
			uint32_t m_ImageCount = 0;

			//std::vector<VkImage> m_DepthImages;
			//std::vector<VkDeviceMemory> m_DepthMemory;

			//VkCommandPool m_CommandPool = VK_NULL_HANDLE;
			//std::vector<VkCommandBuffer> m_CommandBuffers;

			struct Semaphores
			{
				VkSemaphore PresentComplete = nullptr; //Swap Chain present
				VkSemaphore RenderComplete = nullptr; //Command Buffer
			};

			std::vector<Semaphores> m_Semaphores;

			std::vector<VkFence> m_WaitFences;
			//std::shared_ptr<RenderPass> m_Renderpass;
			VkRenderPass m_Renderpass;

			uint32_t m_CurrentBufferIndex = 0;
			uint32_t m_CurrentImageIndex=0;

			std::vector<VkFramebuffer> m_FrameBuffers;

			uint32_t m_QueueIndex = UINT32_MAX;
	};
}