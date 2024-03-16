#include "pch.h"
#include "Vignette.h"
#include "Core/Engine.h"

namespace TRE
{
	Vignette::Vignette() : PostProcessEffect()
	{
		m_BeforeUI = false;

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(8); //Vignette shader
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);	

		SetupUBO();
		SetupShader(PipelineConfig.Shader);
	}

	void Vignette::SetupUBO()
	{
		m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(VignetteUBO)), 0);
	}

	void Vignette::UpdateUBO()
	{
		VignetteUBO UBO{};
		UBO.Color = m_Color;
		UBO.Resolution = glm::vec2(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth(), Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		UBO.Radius = m_Radius;
		UBO.Softness = m_Softness;

		m_UBO->SetData(&UBO, sizeof(VignetteUBO));
	}
}