#include "pch.h"
#include "Silhouette.h"
#include "Core/Engine.h"

namespace TRE
{
	Silhouette::Silhouette() : PostProcessEffect()
	{
		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(16); //Vignette shader
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		SetupUBO();
		SetupShader(PipelineConfig.Shader);
	}

	void Silhouette::SetupUBO()
	{
		m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(SilhouetteUBO)), 0);
	}

	void Silhouette::SetupShader(std::shared_ptr<Shader> shader)
	{
		m_Material = std::make_shared<Material>(shader);
		m_Material->Invalidate();
	}

	void Silhouette::UpdateUBO()
	{
		/*SilhouetteUBO UBO{};
		UBO.Color = m_Color;
		UBO.Resolution = glm::vec2(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth(), Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		UBO.Radius = m_Radius;
		UBO.Softness = m_Softness;

		m_UBO->SetData(&UBO, sizeof(VignetteUBO));*/
	}
}