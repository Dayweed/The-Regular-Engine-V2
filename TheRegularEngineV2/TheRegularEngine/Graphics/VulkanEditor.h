#pragma once
#include "Device.h"

namespace TRE
{
	class VulkanEditor
	{
		public:
			VulkanEditor() = default;
			VulkanEditor(const std::shared_ptr<Device>& LogicalDevice);
			~VulkanEditor();

			void SetUpImgui();

			void BeginFrame();
			void EndFrame();
			void Resize();

			VkDescriptorSet GetDset();

		private:
			std::shared_ptr<Device> m_LogicalDevice;
			std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
			VkDescriptorPool m_DescriptorPool;
			std::vector<VkDescriptorSet> m_DescriptorSets;
	};
}