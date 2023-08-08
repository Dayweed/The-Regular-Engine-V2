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

			void Update();

		private:
			std::shared_ptr<Device> m_LogicalDevice;
			static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
	};
}