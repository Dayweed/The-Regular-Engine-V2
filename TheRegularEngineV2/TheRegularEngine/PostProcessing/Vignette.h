#pragma once

#include "PostRenderer.h"

namespace TRE
{
	struct VignetteUBO
	{
		glm::vec4 Color;
		glm::vec2 Resolution;
		float Radius;
		float Softness;
	};

	class Vignette : public PostProcessEffect
	{
	public:
		void SetupUBO() override;
		void SetupShader(std::shared_ptr<Shader> shader) override;
		void UpdateUBO() override;
		void Render(const std::shared_ptr <Pipeline>& pipeline, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index) override;
	
		void SetColor(const glm::vec4& color) { m_Color = color; }
		void SetRadius(const float radius) { m_Radius = radius; }
		void SetSoftness(const float softness) { m_Softness = softness; }
	private:
		glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		float m_Radius = 1.0f;
		float m_Softness = 0.f;
	};
}