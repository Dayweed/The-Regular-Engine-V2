#pragma once

#include "PostProcessing.h"

namespace TRE
{
	struct DepthBlurUBO
	{
		glm::vec4 Color{ 0.2f,0.2f,0.8f,1.f };
		float Threshold{ 0.0005f };
	};

	class DepthBlur : public PostProcessEffect
	{
	public:
		DepthBlur();

		void SetupUBO() override;
		void UpdateUBO() override;

		//void SetColor(const glm::vec4& color) { m_Color = color; }
		//void SetRadius(const float radius) { m_Radius = radius; }
		//void SetSoftness(const float softness) { m_Softness = softness; }

		//glm::vec4 GetColor() const { return m_Color; }
		//float GetRadius() const { return m_Radius; }
		//float GetSoftness() const { return m_Softness; }
	private:
		//glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		//float m_Radius = 1.0f;
		//float m_Softness = 0.f;
		//glm::vec4 m_Color = glm::vec4(0.2f, 0.2f, 0.8f, 1.f);
		//float m_Threshold = 0.05f;
	};
}