#include "pch.h"
#include "Vignette.h"
#include "Core/Engine.h"

namespace TRE
{
	Vignette::Vignette(const std::shared_ptr<Device>& device) : PostProcessEffect(device)
	{
		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(8); //Vignette shader
		PipelineConfig.EnableCull = false;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);	

		SetupUBO();
		SetupShader(PipelineConfig.Shader);
	}

	void Vignette::SetupUBO()
	{
		m_UBO = std::make_shared<UniformBuffer>(sizeof(VignetteUBO), 0);
	}

	void Vignette::SetupShader(std::shared_ptr<Shader> shader)
	{
		m_Material = std::make_shared<Material>(shader);
		m_Material->Invalidate();
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