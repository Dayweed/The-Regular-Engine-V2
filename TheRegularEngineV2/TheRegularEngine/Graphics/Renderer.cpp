#include "pch.h"
#include "Renderer.h"

namespace TRE
{
	std::shared_ptr<SceneRenderer> Renderer::s_MainRenderer = nullptr;

	void Renderer::SetMainRenderer(const std::shared_ptr<SceneRenderer>& SceneRenderer)
	{
		s_MainRenderer = SceneRenderer;
	}
		
	const std::shared_ptr<SceneRenderer>& Renderer::GetMainRenderer()
	{
		return s_MainRenderer;
	}

	void Renderer::RenderToSwapChain()
	{
	}
}