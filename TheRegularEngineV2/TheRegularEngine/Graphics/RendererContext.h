#pragma once
#include "vulkan/vulkan.h"
#include "PhysicalDevice.h"
#include "Device.h"

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
			bool CheckAPIVersion(uint32_t supportedversion);

			static VkInstance GetVKInstance();
			static std::shared_ptr<RendererContext> Get();
			static std::shared_ptr<Device> GetDevice();
			std::shared_ptr<Device> GetDeviceInternally();

		private:
			static VkInstance m_instance;
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			std::shared_ptr<Device> m_Device;
			VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
	};
}