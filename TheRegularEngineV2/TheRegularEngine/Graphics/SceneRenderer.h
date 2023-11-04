#pragma once
#include "Device.h"
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
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace TRE
{
	struct EditorCamera;
	class Camera;

	struct PushConstant
	{
		glm::mat4 m_Model; //Model to world
	};

	struct UBO
	{
		glm::mat4 m_ProjView{ 1.f }; //World to view to projection
		glm::mat4 m_LightSpaceMatrix {1.f};

		glm::vec3 m_LightPosition{ 0.f, 0.f, 0.f}; //Light position for now will be the camera in world space
		#pragma warning (suppress: 4324) // warning C4324: 'TRE::UBO': structure was padded due to alignment specifier	
		alignas(16) glm::vec4 m_LightColor{ 1.f, 1.f, 1.f, 10.f }; //Light color, w for intensity
		glm::vec4 m_CameraPosition{0.f, 0.f, 0.f, 1.f}; //Camera position in world space
		glm::vec4 m_LightDirection = glm::vec4(glm::normalize(glm::vec3(1.0f, -1.f, 1.f)), 1.f); //Directional Light in world space
		glm::vec4 m_LightDirectionalColor{ 1.f, 1.f, 1.f, 1.f }; //Color for directional light
		glm::vec4 m_LightAmbientColor{ 1.f, 1.f, 1.f, 1.f }; //Color for ambient light

	};

	struct SkyBoxUBO
	{
		glm::mat4 Proj{ 1.f };
		glm::mat4 View{ 1.f };
	};

	struct ShadowUBO
	{
		glm::mat4 view;
		glm::mat4 proj;
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
			void Shutdown();
			void Create();
			void Resize();
			
			void BeginFrame();
			void BeginEditorFrame();
			void EndFrame(bool IsEditorScene);

			void CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass);

			void DebugDrawPass(uint32_t Index);

			//To be reabstracted
			void LoadCubeMap();

		public:
			std::vector<std::shared_ptr<Image2D>> GetColorImages();
			std::shared_ptr<DescriptorPool>& GetDescriptorPool();

		private:
			std::shared_ptr<Device> m_Device;
			std::shared_ptr<CommandBuffer> m_CommandBuffer;

			std::vector<VkFramebuffer> m_FrameBuffer;
			std::shared_ptr<RenderPass> m_RenderPass;
			std::shared_ptr<Pipeline> m_Pipeline;
			
			std::shared_ptr<DescriptorPool> m_DescriptorPool;

			std::vector<std::shared_ptr<Image2D>> m_ColorImages;
			std::vector<std::unique_ptr<Image2D>> m_DepthImages;

			std::shared_ptr<UniformBuffer> m_UBOBuffer;

			std::unique_ptr<DebugRenderer> m_DebugRenderer;

			//Temp for animation Testing
			//std::unique_ptr<AnimationTest> m_Animation;
			//std::shared_ptr<UniformBuffer> m_AnimationUBO;
			//AnimationUBO m_AnimationBuffer;
			//glm::mat4						m_L2W;

			std::shared_ptr<Material>		m_DefaultPBRMaterial;
			ResourceHandle					m_PreviousMaterialHandle;

			//Skybox
			std::shared_ptr<Pipeline> m_SkyboxPipeline;
			std::shared_ptr<VulkanTexture> m_SkyboxTexture;
			std::unique_ptr<Material> m_SkyboxMaterial;
			std::unique_ptr<VertexBuffer> m_SkyboxVertexBuffer;
			std::unique_ptr<IndexBuffer> m_SkyboxIndexBuffer;
			std::shared_ptr<UniformBuffer> m_UBOSkybox;
			//Skybox

			//Shadow
			float zNear = 1.f;
			float zFar = 96.f;
			float depthBiasConstant = 1.25f;
			float depthBiasSlope = 1.75f;
			std::shared_ptr<RenderPass> m_ShadowRenderPass;
			VkDescriptorImageInfo m_ShadowDescriptInfo;
			std::shared_ptr<Pipeline> m_ShadowPipeline;
			std::shared_ptr<Material> m_ShadowMaterial;
			std::shared_ptr<UniformBuffer> m_ShadowUBO;
			uint32_t m_ShadowMapWidth = 1600;
			uint32_t m_ShadowMapHeight = 900;
			VkFramebuffer m_ShadowFramebuffer;
			struct
			{
				VkImage image;
				VkImageView imageview;
				VkDeviceMemory devicememory;
				VkSampler sampler;
			} m_Depth;

			void ShadowPassInit();
			//Shadow
	};
}