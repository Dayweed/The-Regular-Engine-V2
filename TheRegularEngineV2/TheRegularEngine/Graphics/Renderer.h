#pragma once
#include "Device.h"
#include "VulkanEditor.h"
#include "RenderObject.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "Image.h"
#include "Pipeline.h"

namespace TRE
{
	struct PushConstant
	{
		glm::mat4 m_Model; //Model to world
		glm::mat4 m_LightNormal; //Normal matrix per model
	};

	struct UBO
	{
		alignas(16) glm::mat4 m_ProjView{ 1.f }; //World to view to projection
		alignas(16) glm::vec4 m_LightDirection = glm::vec4(glm::normalize(glm::vec3(0.5f, 0.f, 1.f)), 1.f);
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
			void CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass);

			std::vector<std::unique_ptr<Image>>& GetColorImages();
			VkSampler GetSampler();

		private:
			std::shared_ptr<Device> m_Device;
			VkPipelineLayout m_PipelineLayout;

			VkSampler m_Sampler;
			
			std::vector<std::unique_ptr<Image>> m_ColorImages;
			std::vector<std::unique_ptr<Image>> m_DepthImages;

			VkPipeline m_GraphicsPipeline;

			std::unique_ptr<Pipeline> m_Pipeline;
			

			std::vector<VkFramebuffer> m_FrameBuffer;

			std::vector<VkCommandPool> m_CommandPool;
			std::vector<VkCommandBuffer> m_Commandbuffers;
	};
}