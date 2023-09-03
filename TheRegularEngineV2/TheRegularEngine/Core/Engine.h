#pragma once
#include "pch.h"
#include "Window.h"
#include "SystemManager.h"
#include "Graphics/VulkanEditor.h"
#include "Graphics/Renderer.h"

namespace TRE
{
	struct EngineInfo
	{
		WindowConfig WindowConfigurations;
		bool EnableEditor = false;
	};

	class Engine
	{
		public:
			virtual ~Engine();

			Engine(Engine&) = delete;
			void operator=(const Engine&) = delete;

			void RegisterECS();
			void Update();
			virtual void Shutdown();

			template <typename T>
			void RegisterSystems()
			{
				_system_manager->RegisterSystem<T>();
			}

			const std::shared_ptr<Window>& GetWindow();
			const std::shared_ptr<Renderer>& GetRenderer();
			const std::shared_ptr<VulkanEditor>& GetVulkanImgui();
			static Engine& GetInstance();

		protected:
			Engine(const EngineInfo& EngineInfo = EngineInfo());

		private:
			std::shared_ptr<Window> m_Window;
			//std::unique_ptr<SystemManager> m_SystemsManager;
			std::shared_ptr<Renderer> m_Renderer;
			std::shared_ptr<VulkanEditor> m_VulkanEditor;

			EngineInfo m_EngineInfo;

			static Engine* s_Instance;

		public:
			struct QueueFamilies
			{
				int32_t Graphics = -1;
				int32_t Present = -1;

				bool IsComplete() { return ((Graphics != -1) && (Present != -1)); }
			};

			VkInstance m_VKInstance{}; //Data member to hold handle to instance
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			VkDebugUtilsMessengerEXT m_DebugMessage;
			VkPhysicalDevice m_PhysicalDevice; //Auto destroyed when instance is destroyed
			VkDevice m_LogicalDevice;
			VkQueue m_GraphicsQueue;
			VkQueue m_PresentQueue;
			VkSurfaceKHR m_Surface;
			const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
			std::vector<VkImage> m_Images;
			VkExtent2D m_Extent;
			VkFormat m_Format;
			std::vector<VkImageView> m_SwapChainImageViews;
			VkRenderPass m_RenderPass;
			std::vector<VkFramebuffer> m_SwapChainFramebuffers;
			VkCommandPool m_CommandPool;

			std::vector<VkCommandBuffer> m_Commandbuffer;

			std::vector<VkSemaphore> m_ImageAvailable;
			std::vector<VkSemaphore> m_ImageRendered;
			std::vector<VkFence> m_FlightFence;
			uint32_t m_CurrentFrame = 0;

			void InitVulkan();
			void SetupDebugMessage();
			void CreateVulkanInstance();
			bool CheckValidationLayerSupported();
			std::vector<const char*> GetRequiredExtensions() const;
			VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
			//-----Debug-----//
			static 	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType,
				const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData);
			static VkResult CreateDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugmsger);
			void DestroyDebugMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugmessenger, const VkAllocationCallbacks* allocator);
			void PopulateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createinfo);
			//-----Debug-----//

			//-----Physical Device-----//
			void PhysicalDeviceSetup();
			uint32_t GetPhysicalDeviceCount();
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
			//-----Physical Device-----//

			//-----Logical Device-----//
			void CreateLogicalDevice();
			//-----Logical Device-----//

			//-----Queues-----//
			QueueFamilies FindQueueFamilies(VkPhysicalDevice dev);
			//-----Queues-----//

			//-----Window Surface-----//
			void CreateWindowSurface();
			//-----Window Surface-----//

			//-----Swapchain-----//
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
	};

	Engine* CreateApp(); //Implemented by applications projects, used to seperate applications with engine
}