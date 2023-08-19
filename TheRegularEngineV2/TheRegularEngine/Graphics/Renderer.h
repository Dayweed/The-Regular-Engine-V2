#pragma once
#include "Graphics/Device.h"
#include "Graphics/VulkanEditor.h"
#include "RenderObject.h"

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

			std::vector<char> readFile(const std::string& filename);
			VkShaderModule CreateShader(std::vector<char>& code);

			uint32_t FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags);

			std::vector<VkImageView>& GetImageView();
			VkSampler GetSampler();

		private:
			std::shared_ptr<Device> m_Device;
			VkPipelineLayout m_PipelineLayout;

			VkSampler m_Sampler;

			std::vector<VkImage> m_Images;
			std::vector<VkImageView> m_ImageView;
			std::vector<VkDeviceMemory> m_Memory;

			VkRenderPass m_Renderpass;
			VkPipeline m_GraphicsPipeline;
			VkCommandPool m_CommandPool;
			std::vector<VkFramebuffer> m_FrameBuffer;
			std::vector<VkCommandBuffer> m_Commandbuffers;

			VkDescriptorImageInfo DescriptorInfo;

			VkDescriptorSetLayout m_DescriptorLayout;

			VkDescriptorPool m_DescriptorPool;

			std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
	};
}