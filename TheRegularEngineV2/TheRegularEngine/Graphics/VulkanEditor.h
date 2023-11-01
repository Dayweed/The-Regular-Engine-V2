#pragma once
#include "pch.h"
#include "Device.h"
#include "Image.h"
#include "SceneRenderer.h"

namespace TRE
{
	class VulkanEditor
	{
		public:
			VulkanEditor() = default;
			VulkanEditor(const std::shared_ptr<Device>& LogicalDevice);
			~VulkanEditor();

			void SetUpImgui();
			void SetImguiTheme();

			void BeginFrame();
			void EndFrame();
			void Resize();

			void SetEditorSceneDescriptor(const std::shared_ptr<SceneRenderer>& SceneRenderer);
			void SetGameSceneDescriptor(const std::shared_ptr<SceneRenderer>& SceneRenderer);

			VkDescriptorSet GetEditorSceneDescriptor();
			VkDescriptorSet GetGameSceneDescriptor();

		private:
			std::shared_ptr<Device> m_LogicalDevice;
			std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
			VkDescriptorPool m_DescriptorPool;
			
			std::vector<VkDescriptorSet> m_EditorSceneDescriptorSets;
			std::vector<VkDescriptorSet> m_GameSceneDescriptorSets;
	};
}