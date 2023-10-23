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
#include "AnimationTest.h"
#include "ShaderTypes/PBRShader.h"
#include "CommandBuffer.h"

namespace TRE
{
	struct PushConstant
	{
		glm::mat4 m_Model; //Model to world
	};

	struct UBO
	{
		glm::mat4 m_ProjView{ 1.f }; //World to view to projection
		glm::vec3 m_LightPosition{ 0.f, 0.f, 0.f}; //Light position for now will be the camera in world space
		alignas(16) glm::vec4 m_LightColor{ 1.f, 1.f, 1.f, 100.f }; //Light color, w for intensity
		glm::vec4 m_CameraPosition{0.f, 0.f, 0.f, 1.f}; //Camera position in world space
		glm::vec4 m_LightDirection = glm::vec4(glm::normalize(glm::vec3(1.0f, -1.f, 1.f)), 1.f); //Directional Light in world space
		glm::vec4 m_LightAmbientColor{ 1.f, 0.f, 1.f, 1.f }; //Ambient color of the light
	};

	struct AnimationUBO
	{
		glm::mat4 ProjView {1.f};
		glm::mat4 L2W[256];
	};

	class SceneRenderer
	{
		public:
			SceneRenderer(const std::shared_ptr<Device>& Device);
			~SceneRenderer();

			void Initialize();
			void Create();
			void Resize();
			
			void BeginFrame();
			void EndFrame();

			void CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass);

			void DebugDrawPass(uint32_t Index);

		public:
			std::vector<std::unique_ptr<Image>>& GetColorImages();
			std::shared_ptr<DescriptorPool>& GetDescriptorPool();

		private:
			std::shared_ptr<Device> m_Device;
			std::shared_ptr<CommandBuffer> m_CommandBuffer;

			std::unique_ptr<Pipeline> m_Pipeline;
			std::shared_ptr<RenderPass> m_RenderPass;

			std::shared_ptr<DescriptorPool> m_DescriptorPool;

			std::vector<std::unique_ptr<Image>> m_ColorImages;
			std::vector<std::unique_ptr<Image>> m_DepthImages;

			std::vector<VkFramebuffer> m_FrameBuffer;

			std::shared_ptr<UniformBuffer> m_UBOBuffer;

			std::unique_ptr<DebugRenderer> m_DebugRenderer;

			//Temp for animation Testing
			//std::unique_ptr<AnimationTest> m_Animation;
			//std::shared_ptr<UniformBuffer> m_AnimationUBO;
			//AnimationUBO m_AnimationBuffer;
			//glm::mat4						m_L2W;

			std::shared_ptr<Material>		m_DefaultPBRMaterial;
			ResourceHandle					m_PreviousMaterialHandle;
	};
}