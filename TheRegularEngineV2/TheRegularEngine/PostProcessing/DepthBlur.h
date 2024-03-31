#pragma once

#include "PostProcessing.h"

namespace TRE
{
	struct DepthBlurUBO
	{
		float Threshold{ 0.9997f };
	};

	class DepthBlur : public PostProcessEffect
	{
	public:
		DepthBlur();

		void SetupUBO() override;
		void UpdateUBO() override;

		void SetThreshold(float threshold) { m_Threshold = threshold; }
		float GetThreshold() const { return m_Threshold; }
	private:
		float m_Threshold = 0.9997f;
	};
}