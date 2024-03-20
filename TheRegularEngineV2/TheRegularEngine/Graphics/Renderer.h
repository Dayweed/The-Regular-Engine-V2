#pragma once
#include "SceneRenderer.h"

namespace TRE
{
	struct FinalRenderData
	{
		std::unique_ptr<VertexBuffer> VertexBuffer;
		std::unique_ptr<IndexBuffer> IndexBuffer;
		std::shared_ptr<Pipeline> Pipeline;
		std::unique_ptr<Material> Material;
		std::shared_ptr<RenderPass> RenderPass;
	};

	class Renderer
	{
		public:
			static void Init();
			static void Shutdown();
			static void RenderToSwapChain();

		public:
			static void BeginRenderPass(const std::shared_ptr<CommandBuffer>& CommandBuffer, const std::shared_ptr<RenderPass>& Renderpass);
			static void EndRenderPass(const std::shared_ptr<CommandBuffer>& CommandBuffer);
			static void BeginFrame();
			static void EndFrame();
			static void BindPipeline(const std::shared_ptr<CommandBuffer>& CommandBuffer, const std::shared_ptr<Pipeline>& Pipeline, bool IsCompute = false);

			static void SetSkyboxEnvironment(std::string texture0, std::string texture1, std::string texture2, std::string texture3, std::string texture4, std::string texture5);
			static void SetGamma(float Gamma);
			static float GetGammaValue();

		public:
			static std::shared_ptr<Skybox> GetSkybox();

		private:
			static std::shared_ptr<SceneRenderer> s_MainRenderer;
			static std::shared_ptr<CommandBuffer> m_CommandBuffer;
			static FinalRenderData* s_FinalRenderData;
			static std::shared_ptr<Skybox> m_SkyboxEnvironment;
			static float s_GammaValue;
	};
}