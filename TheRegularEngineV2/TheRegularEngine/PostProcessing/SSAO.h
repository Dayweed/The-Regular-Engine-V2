#pragma once

#include "PostProcessing.h"

namespace TRE
{
	struct SSAOUBO
	{
		glm::vec2 m_ScreenSize;
		float m_Radius = 0.5f;
		float m_Bias = 0.025f;
		float m_Intensity = 1.f;
		float m_MaxDistance = 1.f;
	};

	class SSAO : public PostProcessEffect
	{
	public:
		SSAO();

		void SetupUBO() override;
		void UpdateUBO() override;

		void SetRadius(float Radius) { m_Radius = Radius; }
		void SetBias(float Bias) { m_Bias = Bias; }
		void SetIntensity(float Intensity) { m_Intensity = Intensity; }
		void SetMaxDistance(float MaxDistance) { m_MaxDistance = MaxDistance; }

		float GetRadius() const { return m_Radius; }
		float GetBias() const { return m_Bias; }
		float GetIntensity() const { return m_Intensity; }
		float GetMaxDistance() const { return m_MaxDistance; }

	private:
		float m_Radius = 0.01f;//0.003f;
		float m_Bias = 0.0005f;
		float m_Intensity = 5.f;
		float m_MaxDistance = 0.0025f;
	};
}