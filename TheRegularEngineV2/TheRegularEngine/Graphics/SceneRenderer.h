#pragma once
#include "Device.h"
#include "RenderObject.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "Image.h"
#include "Pipeline.h"
#include "Material.h"
#include "DebugRenderer.h"
#include "ShaderTypes/PBRShader.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Resource/ResourceManager.h"
#include "UIRenderer.h"
#include "PostProcessing/PostProcessing.h"
#include "ParticleRenderer.h"
#include "Skybox.h"

namespace TRE
{
	class Ent;
	typedef std::shared_ptr<Ent> Entity;
	class BaseCamera;
	class FontRenderer;

	//To be used by normmal geometry and animation geometry
	struct PushConstantGeometry
	{
		glm::mat4 m_Model;
		bool m_DrawShadow{ true };
	};

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
		float m_ShadowIntensity{ 0.85f }; //Shadow intensity
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

	struct DepthUBO
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct IDUBO
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	class SceneRenderer
	{
		public:
			SceneRenderer(bool IsEditorScene);
			~SceneRenderer();

			void Initialize();
			void ShadowPassInit();
			void DepthPrepassInit();
			void IDPrepassInit();
			void Shutdown();
			void Create();
			void Resize();
			
			void BeginFrame();
			void BeginEditorFrame();
			void EndFrame();

			void CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass);

			void ShadowPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort);
			void DepthPrepass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort);
			void IDPrepass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort);
			void GeometryPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort);
			void GeometryAnimationPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort);
			void DebugDrawPass(uint32_t Index);
			void SkyBoxPass(uint32_t Index);
			void Sprite3DPass(uint32_t Index);

		public:
			std::vector<std::shared_ptr<Image2D>> GetColorImages();
			std::shared_ptr<DescriptorPool>& GetDescriptorPool();

		private:
			bool ShadowFrustumCheck(const BaseCamera& baseCamera);
			void RecreateShadowAABB(const std::array<glm::vec3, 8>& cameraFrustum);
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

			std::shared_ptr<Material>		m_DefaultPBRMaterial;
			std::shared_ptr<Material>		m_DefaultAnimationPBRMaterial;
			ResourceHandle					m_PreviousMaterialHandle;

			//Skybox
			std::shared_ptr<Pipeline> m_SkyboxPipeline;
			std::shared_ptr<UniformBuffer> m_UBOSkybox;
			//Skybox

			//Shadow
			float depthBiasConstant = 1.25f;
			float depthBiasSlope = 1.75f;
			std::shared_ptr<Image2D> m_ShadowImages;
			std::shared_ptr<RenderPass> m_ShadowRenderPass;
			VkDescriptorImageInfo m_ShadowDescriptInfo;
			std::shared_ptr<Pipeline> m_ShadowPipeline;
			std::shared_ptr<Material> m_ShadowMaterial;
			std::shared_ptr<Pipeline> m_ShadowAnimationPipeline;
			std::shared_ptr<UniformBuffer> m_ShadowUBO;
			uint32_t m_ShadowMapWidth = 1600;
			uint32_t m_ShadowMapHeight = 900;
			VkFramebuffer m_ShadowFramebuffer;
			float m_ShadowAABBPadding = 10.f;

			//Depth Prepass
			std::shared_ptr<Image2D> m_DepthPrepassImages;
			std::shared_ptr<RenderPass> m_DepthPrepassRenderPass;
			VkDescriptorImageInfo m_DepthPrepassDescriptInfo;
			std::shared_ptr<Pipeline> m_DepthPrepassPipeline;
			std::shared_ptr<Pipeline> m_DepthPrepassAnimationPipeline;
			std::shared_ptr<Material> m_DepthPrepassMaterial;
			std::shared_ptr<UniformBuffer> m_DepthPrepassUBO;
			uint32_t m_DepthPrepassMapWidth = 1600;
			uint32_t m_DepthPrepassMapHeight = 900;
			VkFramebuffer m_DepthPrepassFramebuffer;

			//ID Prepass
			std::shared_ptr<Image2D> m_IDPrepassImages;
			std::shared_ptr<RenderPass> m_IDPrepassRenderPass;
			VkDescriptorImageInfo m_IDPrepassDescriptInfo;
			std::shared_ptr<Pipeline> m_IDPrepassPipeline;
			std::shared_ptr<Pipeline> m_IDPrepassAnimationPipeline;
			std::shared_ptr<Material> m_IDPrepassMaterial;
			std::shared_ptr<UniformBuffer> m_IDPrepassUBO;
			uint32_t m_IDPrepassMapWidth = 1600;
			uint32_t m_IDPrepassMapHeight = 900;
			VkFramebuffer m_IDPrepassFramebuffer;

			//Game
			glm::vec3 m_ShadowAABBMin;
			glm::vec3 m_ShadowAABBMax;
			glm::vec3 m_ShadowRenderPoint;
			glm::mat4 m_ShadowView;
			glm::mat4 m_ShadowProj;

			//Editor viewport
			glm::vec3 m_EditorShadowAABBMin;
			glm::vec3 m_EditorShadowAABBMax;
			glm::vec3 m_EditorShadowRenderPoint;
			glm::mat4 m_EditorShadowView;
			glm::mat4 m_EditorShadowProj;
			float m_EditorShadowRatio = 0.05f;

			//AnimationPass
			std::shared_ptr<Pipeline> m_AnimationPipeline;
			glm::mat4 m_L2W;

			//UI Renderer
			std::shared_ptr<UIRenderer> m_UIRenderer;
			glm::mat4 m_ProjView3D;

			//Font Renderer
			std::shared_ptr<FontRenderer> m_FontRenderer;

			//Particle Renderer
			std::shared_ptr<ParticleRenderer> m_ParticleRenderer;
			std::shared_ptr<UniformBuffer> m_ParticleUBO2D;
			std::shared_ptr<UniformBuffer> m_ParticleUBO3D;

			//Sprite 3D Renderer
			std::shared_ptr<Pipeline> m_Sprite3DPipeline;
			std::shared_ptr<UniformBuffer> m_Sprite3DUBO;
			std::shared_ptr<VertexBuffer> m_Sprite3DVertexBuffer;
			std::shared_ptr<IndexBuffer> m_Sprite3DIndexBuffer;

			bool m_IsEditorScene = false;
	};
}