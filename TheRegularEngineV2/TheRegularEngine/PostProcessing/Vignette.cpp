#include "pch.h"
#include "Vignette.h"
#include "Core/Engine.h"

namespace TRE
{
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
		UBO.Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		UBO.Resolution = glm::vec2(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth(), Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		UBO.Radius = 1.0f;
		UBO.Softness = 0.f;

		m_UBO->SetData(&UBO, sizeof(VignetteUBO));
	}

	void Vignette::Render(const std::shared_ptr <Pipeline>& pipeline, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index)
	{
		m_Material->UpdateForRendering(m_UBO, index);
		vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &m_Material->GetDescriptor(index), 0, NULL);
	}
}