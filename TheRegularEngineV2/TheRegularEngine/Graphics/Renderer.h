#pragma once
#include "SceneRenderer.h"

namespace TRE
{
	class Renderer
	{
		public:
			static void Init();

			static void RenderToSwapChain();
			static void SetMainRenderer(const std::shared_ptr<SceneRenderer>& SceneRenderer);
			static const std::shared_ptr<SceneRenderer>& GetMainRenderer();

		private:
			static std::shared_ptr<SceneRenderer> s_MainRenderer;
	};
}