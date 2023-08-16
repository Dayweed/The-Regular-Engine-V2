#pragma once
#include "Graphics/Device.h"
#include "Graphics/VulkanEditor.h"

namespace TRE
{
	class Renderer
	{
		public:
			Renderer(const std::shared_ptr<Device>& Device);
			~Renderer();
			void Initialize();
			void Shutdown();

			void BeginFrame();
			void DrawFrame();

			std::vector<char> readFile(const std::string& filename);
			VkShaderModule CreateShader(std::vector<char>& code);

			uint32_t FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags);

			VkImageView GetImageView();

		private:
			std::shared_ptr<Device> m_Device;
			VkPipeline m_GraphicsPipeline;
			VkPipelineLayout m_PipelineLayout;

			VkRenderPass m_Renderpass;
			VkFramebuffer m_FrameBuffer;
			VkSampler m_Sampler;

			VkImage m_Image;
			VkImageView m_ImageView;
			VkDeviceMemory m_Memory;

			VkDescriptorImageInfo DescriptorInfo;
			VkDescriptorSet m_DescriptorSet;
			VkDescriptorSetLayout m_DescriptorLayout;

			friend class VulkanEditor;
	};
}