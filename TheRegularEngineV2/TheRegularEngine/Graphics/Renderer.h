#pragma once
#include "Graphics/Device.h"
#include "Graphics/VulkanEditor.h"
#include "RenderObject.h"
#include "Descriptor.h"
#include "RenderPass.h"

namespace TRE
{
	struct PushConstant
	{
		glm::mat4 m_Model; //Model to world
		glm::mat4 m_ProjView; //World to view to projection
	};

	struct UBO
	{
		alignas(16) glm::mat4 m_ProjView{ 1.f };
		alignas(16) glm::vec4 m_LightDirection = glm::vec4(glm::normalize(glm::vec3(1.f, -3.f, -1.f)), 1.f);
	};

	class Renderer
	{
		public:
			Renderer(const std::shared_ptr<Device>& Device);
			~Renderer();

			void Initialize();
			void Create();
			void Resize();
			void Shutdown();
			void BeginFrame();

			std::vector<char> readFile(const std::string& filename);
			VkShaderModule CreateShader(std::vector<char>& code);


			std::vector<VkImageView>& GetImageView();
			VkSampler GetSampler();

		private:
			uint32_t FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags);
		private:
			std::shared_ptr<Device> m_Device;
			VkPipelineLayout m_PipelineLayout;

			VkSampler m_Sampler;

			std::vector<VkImage> m_Images;
			std::vector<VkImageView> m_ImageView;
			std::vector<VkDeviceMemory> m_Memory;

			std::shared_ptr<RenderPass> m_Renderpass;
			VkPipeline m_GraphicsPipeline;
			std::vector<VkFramebuffer> m_FrameBuffer;

			std::vector<VkCommandPool> m_CommandPool;
			std::vector<VkCommandBuffer> m_Commandbuffers;

			std::unique_ptr<DescriptorPool> m_DescriptorPool;
			std::vector<std::unique_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSets;

			std::vector<std::unique_ptr<Buffer>> m_UBOBuffers{};

	};
}