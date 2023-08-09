#pragma once
#include "TREIncludes.h"
#include "pch.h"

namespace TRE
{
	class VulkanEditor
	{
		public:
			VulkanEditor() = default;
			VulkanEditor(std::shared_ptr<Device> LogicalDevice);
			~VulkanEditor();

			void BeginFrame();
			void EndFrame();

		private:
			std::shared_ptr<Device> m_LogicalDevice;
			static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
			VkDescriptorPool m_DescriptorPool;
	};
}