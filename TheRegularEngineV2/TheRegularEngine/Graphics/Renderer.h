#pragma once
#include "SceneRenderer.h"

namespace TRE
{
	class Renderer
	{
		public:
			static void SetMainRenderer(const std::shared_ptr<SceneRenderer>& SceneRenderer);
			static const std::shared_ptr<SceneRenderer>& GetMainRenderer();
			static void RenderToSwapChain();

		private:
			static std::shared_ptr<SceneRenderer> s_MainRenderer;
	};
}