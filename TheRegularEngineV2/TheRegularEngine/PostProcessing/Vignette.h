#pragma once

#include "PostProcessing.h"

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
		Vignette(const std::shared_ptr<Device>& device);

		void SetupUBO() override;
		void SetupShader(std::shared_ptr<Shader> shader) override;
		void UpdateUBO() override;
	
		void SetColor(const glm::vec4& color) { m_Color = color; }
		void SetRadius(const float radius) { m_Radius = radius; }
		void SetSoftness(const float softness) { m_Softness = softness; }

		glm::vec4 GetColor() const { return m_Color; }
		float GetRadius() const { return m_Radius; }
		float GetSoftness() const { return m_Softness; }
	private:
		glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		float m_Radius = 1.0f;
		float m_Softness = 0.f;
	};
}