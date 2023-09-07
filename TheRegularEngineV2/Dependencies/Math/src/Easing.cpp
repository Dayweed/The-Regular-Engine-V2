/*!
	@file      Easing.cpp
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains a number of definitions of commonly used
			   easing functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#include "Easing.h"

#define _USE_MATH_DEFINES
#include <cmath>
// with the define, M_PI and other math constants are now defined

#define PI static_cast<float>(M_PI)
#define PI_2 static_cast<float>(M_PI_2)
#define PIx2 static_cast<float>(M_PI + M_PI)

namespace Mathf::Easing // nested namespace definitions! :D
{
	float EaseLinear(float start, float end, float value)
	{
		return (1.f - value) * start + value * end;
	}
	
	float EaseInQuad(float start, float end, float value)
	{
		end -= start;
		return end * value * value + start;
	}

	float EaseOutQuad(float start, float end, float value)
	{
		end -= start;
		return -end * value * (value - 2) + start;
	}

	float EaseInOutQuad(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return end * 0.5f * value * value + start;
		value--;
		return -end * 0.5f * (value * (value - 2) - 1) + start;
	}

	float EaseInCubic(float start, float end, float value)
	{
		end -= start;
		return end * value * value * value + start;
	}

	float EaseOutCubic(float start, float end, float value)
	{
		value--;
		end -= start;
		return end * (value * value * value + 1) + start;
	}

	float EaseInOutCubic(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return end * 0.5f * value * value * value + start;
		value -= 2;
		return end * 0.5f * (value * value * value + 2) + start;
	}
	
	float EaseInQuart(float start, float end, float value)
	{
		end -= start;
		return end * value * value * value * value + start;
	}
	
	float EaseOutQuart(float start, float end, float value)
	{
		value--;
		end -= start;
		return -end * (value * value * value * value - 1) + start;
	}

	float EaseInOutQuart(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return end * 0.5f * value * value * value * value + start;
		value -= 2;
		return -end * 0.5f * (value * value * value * value - 2) + start;
	}
	
	float EaseInQuint(float start, float end, float value)
	{
		end -= start;
		return end * value * value * value * value * value + start;
	}
	
	float EaseOutQuint(float start, float end, float value)
	{
		value--;
		end -= start;
		return end * (value * value * value * value * value + 1) + start;
	}

	float EaseInOutQuint(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return end * 0.5f * value * value * value * value * value + start;
		value -= 2;
		return end * 0.5f * (value * value * value * value * value + 2) + start;
	}

	float EaseInSine(float start, float end, float value)
	{
		end -= start;
		return -end * cosf(value * PI_2) + end + start;
	}

	float EaseOutSine(float start, float end, float value)
	{
		end -= start;
		return end * sinf(value * PI_2) + start;
	}

	float EaseInOutSine(float start, float end, float value)
	{
		end -= start;
		return -end * 0.5f * (cosf(PI * value) - 1) + start;
	}

	float EaseInExpo(float start, float end, float value)
	{
		end -= start;
		return end * powf(2, 10 * (value - 1)) + start;
	}
	
	float EaseOutExpo(float start, float end, float value)
	{
		end -= start;
		return end * (-powf(2, -10 * value) + 1) + start;
	}

	float EaseInOutExpo(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return end * 0.5f * powf(2, 10 * (value - 1)) + start;
		value--;
		return end * 0.5f * (-powf(2, -10 * value) + 2) + start;
	}

	float EaseInCirc(float start, float end, float value)
	{
		end -= start;
		return -end * (sqrtf(1 - value * value) - 1) + start;
	}

	float EaseOutCirc(float start, float end, float value)
	{
		value--;
		end -= start;
		return end * sqrtf(1 - value * value) + start;
	}
	
	float EaseInOutCirc(float start, float end, float value)
	{
		value /= .5f;
		end -= start;
		if (value < 1) return -end * 0.5f * (sqrtf(1 - value * value) - 1) + start;
		value -= 2;
		return end * 0.5f * (sqrtf(1 - value * value) + 1) + start;
	}

	float EaseOutBounce(float start, float end, float value)
	{
		value /= 1.f;
		end -= start;
		if (value < (1 / 2.75f))
		{
			return end * (7.5625f * value * value) + start;
		}
		else if (value < (2 / 2.75f))
		{
			value -= (1.5f / 2.75f);
			return end * (7.5625f * (value)*value + .75f) + start;
		}
		else if (value < (2.5 / 2.75))
		{
			value -= (2.25f / 2.75f);
			return end * (7.5625f * (value)*value + .9375f) + start;
		}
		else
		{
			value -= (2.625f / 2.75f);
			return end * (7.5625f * (value)*value + .984375f) + start;
		}
	}

	float EaseInBounce(float start, float end, float value)
	{
		end -= start;
		float d = 1.f;
		return end - EaseOutBounce(0, end, d - value) + start;
	}

	float EaseInOutBounce(float start, float end, float value)
	{
		end -= start;
		float d = 1.f;
		if (value < d * 0.5f) return EaseInBounce(0, end, value * 2) * 0.5f + start;
		else return EaseOutBounce(0, end, value * 2 - d) * 0.5f + end * 0.5f + start;
	}

	float EaseInBack(float start, float end, float value)
	{
		end -= start;
		value /= 1;
		float s = 1.70158f;
		return end * (value)*value * ((s + 1) * value - s) + start;
	}

	float EaseOutBack(float start, float end, float value)
	{
		float s = 1.70158f;
		end -= start;
		value = (value)-1;
		return end * ((value)*value * ((s + 1) * value + s) + 1) + start;
	}

	float EaseInOutBack(float start, float end, float value)
	{
		float s = 1.70158f;
		end -= start;
		value /= .5f;
		if ((value) < 1)
		{
			s *= (1.525f);
			return end * 0.5f * (value * value * (((s)+1) * value - s)) + start;
		}
		value -= 2;
		s *= (1.525f);
		return end * 0.5f * ((value)*value * (((s)+1) * value + s) + 2) + start;
	}

	float EaseInElastic(float start, float end, float value)
	{
		end -= start;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value == 0) return start;

		if ((value /= d) == 1) return start + end;

		if (a == 0.f || a < fabs(end))
		{
			a = end;
			s = p / 4;
		}
		else
		{
			s = p / (PIx2) * asinf(end / a);
		}

		return -(a * powf(2, 10 * (value -= 1)) * sinf((value * d - s) * (PIx2) / p)) + start;
	}

	float EaseOutElastic(float start, float end, float value)
	{
		end -= start;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value == 0) return start;

		if ((value /= d) == 1) return start + end;

		if (a == 0.f || a < fabs(end))
		{
			a = end;
			s = p * 0.25f;
		}
		else
		{
			s = p / (PIx2) * asinf(end / a);
		}

		return (a * powf(2, -10 * value) * sinf((value * d - s) * (PIx2) / p) + end + start);
	}

	float EaseInOutElastic(float start, float end, float value)
	{
		end -= start;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value == 0) return start;

		if ((value /= d * 0.5f) == 2) return start + end;

		if (a == 0.f || a < fabs(end))
		{
			a = end;
			s = p / 4;
		}
		else
		{
			s = p / (PIx2) * asinf(end / a);
		}

		if (value < 1) return -0.5f * (a * powf(2, 10 * (value -= 1)) * sinf((value * d - s) * (PIx2) / p)) + start;
		return a * powf(2, -10 * (value -= 1)) * sinf((value * d - s) * (PIx2) / p) * 0.5f + end + start;
	}
}
