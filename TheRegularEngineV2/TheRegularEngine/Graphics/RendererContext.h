#pragma once
#include "Device.h"
#include "vulkan/vulkan.h"

namespace TRE
{
	#ifdef _DEBUG
		static const bool EnableValidationLayer = true;
	#else
		static const bool EnableValidationLayer = false;
	#endif

	class RendererContext
	{
		public:
			RendererContext();
			~RendererContext();

			void Initialize();

			std::shared_ptr<PhysicalDevice>& GetPhysicalDeviceInternally();
			std::shared_ptr<Device>& GetDeviceInternally();
			VkSurfaceKHR GetSurface();

			static VkInstance GetVKInstance();
			static uint32_t GetFramesInFlight();
			static std::shared_ptr<RendererContext> Get();
			static std::shared_ptr<Device> GetDevice();
			static std::shared_ptr<PhysicalDevice> GetPhysicalDevice();

		private:
			bool CheckAPIVersion(uint32_t supportedversion);

		private:
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			std::shared_ptr<Device> m_Device;

		private:
			static VkInstance s_Instance;
			static uint32_t s_FramesInFlight;
			VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
			VkSurfaceKHR m_Surface;
	};
}