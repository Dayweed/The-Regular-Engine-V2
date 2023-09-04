#pragma once

namespace TRE
{
	struct QueueFamilies
	{
		int32_t Graphics = -1;
		int32_t Compute = -1;

		bool IsComplete() { return ((Graphics != -1) && (Compute != -1)); }
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities{};
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class PhysicalDevice
	{
	public:
		PhysicalDevice(VkSurfaceKHR Surface);
		~PhysicalDevice();

		QueueFamilies GetQueueFamilies();
		QueueFamilies GetQueueFamilies(int flags);
		VkFormat GetDepthFormat();
		VkPhysicalDevice GetPhysicalDevice() const;
		VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
		VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();
		QueueFamilies FindQueueFamilies(VkPhysicalDevice dev);

	private:
		bool IsExtensionSupported(const std::string& Extension);
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
		SwapChainDetails QuerySwapChainSupprt(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);


	private:
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDeviceFeatures m_Features;
		VkPhysicalDeviceMemoryProperties m_MemoryProperties;

		QueueFamilies m_QueueFamilies;
		std::vector <VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

		std::unordered_set<std::string> m_SupportedExtensions;
		VkFormat m_DepthFormat;
	};

	class Device
	{
		public:
			Device(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures Features);
			~Device();

			void Destroy();
			VkCommandBuffer AllocateCommandBuffer(bool BeginBuffer);
			VkCommandBuffer AllocateSecondaryCommandBuffer();
			void SubmitCommands(VkCommandBuffer CommandBuffer);

			const std::shared_ptr<PhysicalDevice>& GetPhysicalDevice() const;
			VkDevice GetLogicalDevice() const;
			VkQueue GetGraphicsQ();
			VkQueue GetComputeQ();
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			
			uint32_t FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags);
		
		private:
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;

		private:
			VkDevice m_LogicalDevice;
			VkPhysicalDeviceFeatures m_EnabledFeatures;
			VkCommandPool m_CommandPool;
			VkCommandPool m_ComputeCommandPool;

			VkQueue m_GraphicsQ;
			VkQueue m_ComputeQ;
	};
}