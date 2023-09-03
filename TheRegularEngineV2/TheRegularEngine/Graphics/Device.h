#pragma once
#include "PhysicalDevice.h"

namespace TRE
{
	class Device
	{
		public:
			Device(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures Features = VkPhysicalDeviceFeatures());
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