#pragma once
#include "SceneRenderer.h"

namespace TRE
{
	struct FinalRenderData
	{
		std::unique_ptr<Buffer> VertexBuffer;
		std::unique_ptr<Buffer> IndexBuffer;
		std::unique_ptr<Pipeline> Pipeline;
		std::unique_ptr<Material> Material;
		std::shared_ptr<RenderPass> RenderPass;
		VkDescriptorImageInfo ImageInfo;
	};

	class Renderer
	{
		public:
			static void Init();
			static void Shutdown();
			static void RenderToSwapChain();

		private:
			static std::shared_ptr<SceneRenderer> s_MainRenderer;
			static std::shared_ptr<CommandBuffer> m_CommandBuffer;
			static FinalRenderData* s_FinalRenderData;
	};
}