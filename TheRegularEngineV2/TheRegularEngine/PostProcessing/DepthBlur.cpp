#include "pch.h"
#include "DepthBlur.h"
#include "Core/Engine.h"

namespace TRE
{
	DepthBlur::DepthBlur() : PostProcessEffect()
	{
		m_BeforeUI = true;
		m_Active = false;

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(17); //Silhoutte shader
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		SetupUBO();
		SetupShader(PipelineConfig.Shader);
	}

	void DepthBlur::SetupUBO()
	{
		m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(DepthBlurUBO)), 0);
	}

	void DepthBlur::UpdateUBO()
	{
		DepthBlurUBO UBO{};
		UBO.Threshold = m_Threshold;

		m_UBO->SetData(&UBO, sizeof(DepthBlurUBO));
	}
}