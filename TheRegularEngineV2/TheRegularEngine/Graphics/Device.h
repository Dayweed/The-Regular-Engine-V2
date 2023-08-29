#pragma once
#include "PhysicalDevice.h"

namespace TRE
{
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

			uint32_t FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags);

		private:
			VkDevice m_LogicalDevice;
			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			VkPhysicalDeviceFeatures m_EnabledFeatures;
			VkCommandPool m_CommandPool;
			VkCommandPool m_ComputeCommandPool;

			VkQueue m_GraphicsQ;
			VkQueue m_ComputeQ;
	};
}