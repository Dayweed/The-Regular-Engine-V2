#pragma once
#include "TREIncludes.h"
#include "pch.h"

namespace TRE
{
	class VulkanEditor
	{
		public:
			VulkanEditor() = default;
			VulkanEditor(const std::shared_ptr<Device>& LogicalDevice);
			~VulkanEditor();

			void BeginFrame();
			void EndFrame();

			VkDescriptorSet GetFinalImageInternal();
			static VkDescriptorSet GetFinalImage();

		private:
			std::shared_ptr<Device> m_LogicalDevice;
			std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;

			VkDescriptorSetLayout m_DescriptorSetLayout;
			VkDescriptorPool m_DescriptorPool;
			std::vector<VkDescriptorSet> m_DescriptorSets;
			VkSampler m_TextureSampler;
	};
}