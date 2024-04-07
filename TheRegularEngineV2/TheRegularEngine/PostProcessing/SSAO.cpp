#include "pch.h"
#include "SSAO.h"
#include "Core/Engine.h"

namespace TRE
{
	SSAO::SSAO() : PostProcessEffect()
	{
		m_BeforeUI = true;

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(20); //SSAO shader
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		SetupUBO();
		SetupShader(PipelineConfig.Shader);
	}

	void SSAO::SetupUBO()
	{
		m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(SSAOUBO)), 0);
	}

	void SSAO::UpdateUBO()
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		SSAOUBO UBO{};
		UBO.m_ScreenSize = glm::vec2(SC->GetWidth(), SC->GetHeight());
		UBO.m_Radius = m_Radius;
		UBO.m_Bias = m_Bias;
		UBO.m_Intensity = m_Intensity;
		UBO.m_MaxDistance = m_MaxDistance;

		m_UBO->SetData(&UBO, sizeof(SSAOUBO));
	}
}