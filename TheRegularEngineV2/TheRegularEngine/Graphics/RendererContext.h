#pragma once
#include "vulkan/vulkan.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"

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
			void Initialize();
			bool CheckAPIVersion(uint32_t supportedversion);
			void LoadDebugExtensions(VkInstance instance);

			static VkInstance GetVKInstance();
			std::shared_ptr<PhysicalDevice> GetPhysicalDevice();
			std::shared_ptr<LogicalDevice> GetLogicalDevice();

		private:
			static VkInstance m_instance;
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			std::shared_ptr<LogicalDevice> m_LogicalDevice;
			VkPipelineCache m_PipelineCache = nullptr;
			VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;
	};
}