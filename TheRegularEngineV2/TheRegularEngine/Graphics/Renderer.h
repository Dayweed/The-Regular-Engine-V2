#pragma once
#include "Device.h"
#include "VulkanEditor.h"
#include "RenderObject.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "Image.h"
#include "Pipeline.h"
#include "Material.h"
#include "DebugRenderer.h"

namespace TRE
{
	struct PushConstant
	{
		glm::mat4 m_Model; //Model to world
	};

	struct UBO
	{
		alignas(16) glm::mat4 m_ProjView{ 1.f }; //World to view to projection
		alignas(16) glm::vec4 m_LightDirection = glm::vec4(glm::normalize(glm::vec3(0.0f, 0.f, 1.f)), 1.f);
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

			void CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass);

			void DebugDrawPass(VkCommandBuffer CommandBuffer);

		public:
			std::vector<std::unique_ptr<Image>>& GetColorImages();
			std::shared_ptr<DescriptorPool>& GetDescriptorPool();

		private:
			std::shared_ptr<Device> m_Device;

		private:
			std::unique_ptr<Pipeline> m_Pipeline;
			std::shared_ptr<RenderPass> m_RenderPass;

			std::shared_ptr<DescriptorPool> m_DescriptorPool;

			std::vector<std::unique_ptr<Image>> m_ColorImages;
			std::vector<std::unique_ptr<Image>> m_DepthImages;

			std::vector<VkCommandPool> m_CommandPool;
			std::vector<VkCommandBuffer> m_Commandbuffers;
			std::vector<VkFramebuffer> m_FrameBuffer;

			std::shared_ptr<UniformBuffer> m_UBOBuffer;

			std::unique_ptr<DebugRenderer> m_DebugRenderer;
	};
}