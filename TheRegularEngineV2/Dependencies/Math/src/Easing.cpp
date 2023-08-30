#include "Easing.h"

#define _USE_MATH_DEFINES
#include <cmath>
// with the define, M_PI and other math constants are now defined

#define PI static_cast<float>(M_PI)
#define PI_2 static_cast<float>(M_PI_2)
#define PIx2 static_cast<float>(M_PI + M_PI)

namespace Mathf::Easing // nested namespace definitions! :D
{
	/* !
	@function  EaseLinear

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			linearly.

	*//*__________________________________________________________________________*/
	float EaseLinear(float start_, float end_, float value_)
	{
		return (1.f - value_) * start_ + value_ * end_;
	}

	/* !
	@function  EaseInQuad

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards quadratically.

	*//*__________________________________________________________________________*/
	float EaseInQuad(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * value_ * value_ + start_;
	}

	/* !
	@function  EaseOutQuad

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards quadratically.

	*//*__________________________________________________________________________*/
	float EaseOutQuad(float start_, float end_, float value_)
	{
		end_ -= start_;
		return -end_ * value_ * (value_ - 2) + start_;
	}

	/* !
	@function  EaseInOutQuad

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards quadratically.

	*//*__________________________________________________________________________*/
	float EaseInOutQuad(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return end_ * 0.5f * value_ * value_ + start_;
		value_--;
		return -end_ * 0.5f * (value_ * (value_ - 2) - 1) + start_;
	}

	/* !
	@function  EaseInCubic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards cubically.

	*//*__________________________________________________________________________*/
	float EaseInCubic(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * value_ * value_ * value_ + start_;
	}

	/* !
	@function  EaseOutCubic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards cubically.

	*//*__________________________________________________________________________*/
	float EaseOutCubic(float start_, float end_, float value_)
	{
		value_--;
		end_ -= start_;
		return end_ * (value_ * value_ * value_ + 1) + start_;
	}

	/* !
	@function  EaseInOutCubic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards cubically.

	*//*__________________________________________________________________________*/
	float EaseInOutCubic(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return end_ * 0.5f * value_ * value_ * value_ + start_;
		value_ -= 2;
		return end_ * 0.5f * (value_ * value_ * value_ + 2) + start_;
	}

	/* !
	@function  EaseInQuart

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards quartically.

	*//*__________________________________________________________________________*/
	float EaseInQuart(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * value_ * value_ * value_ * value_ + start_;
	}

	/* !
	@function  EaseOutQuart

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards quartically.

	*//*__________________________________________________________________________*/
	float EaseOutQuart(float start_, float end_, float value_)
	{
		value_--;
		end_ -= start_;
		return -end_ * (value_ * value_ * value_ * value_ - 1) + start_;
	}

	/* !
	@function  EaseInOutQuart

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards quartically.

	*//*__________________________________________________________________________*/
	float EaseInOutQuart(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return end_ * 0.5f * value_ * value_ * value_ * value_ + start_;
		value_ -= 2;
		return -end_ * 0.5f * (value_ * value_ * value_ * value_ - 2) + start_;
	}

	/* !
	@function  EaseInQuint

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards quintically.

	*//*__________________________________________________________________________*/
	float EaseInQuint(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * value_ * value_ * value_ * value_ * value_ + start_;
	}

	/* !
	@function  EaseOutQuint

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards quintically.

	*//*__________________________________________________________________________*/
	float EaseOutQuint(float start_, float end_, float value_)
	{
		value_--;
		end_ -= start_;
		return end_ * (value_ * value_ * value_ * value_ * value_ + 1) + start_;
	}

	/* !
	@function  EaseInOutQuint

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards quintically.

	*//*__________________________________________________________________________*/
	float EaseInOutQuint(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return end_ * 0.5f * value_ * value_ * value_ * value_ * value_ + start_;
		value_ -= 2;
		return end_ * 0.5f * (value_ * value_ * value_ * value_ * value_ + 2) + start_;
	}

	/* !
	@function  EaseInSine

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards sinusoidally.

	*//*__________________________________________________________________________*/
	float EaseInSine(float start_, float end_, float value_)
	{
		end_ -= start_;
		return -end_ * cosf(value_ * PI_2) + end_ + start_;
	}

	/* !
	@function  EaseOutSine

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards sinusoidally.

	*//*__________________________________________________________________________*/
	float EaseOutSine(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * sinf(value_ * PI_2) + start_;
	}

	/* !
	@function  EaseInOutSine

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards sinusoidally.

	*//*__________________________________________________________________________*/
	float EaseInOutSine(float start_, float end_, float value_)
	{
		end_ -= start_;
		return -end_ * 0.5f * (cosf(PI * value_) - 1) + start_;
	}

	/* !
	@function  EaseInExpo

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards exponentially.

	*//*__________________________________________________________________________*/
	float EaseInExpo(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * powf(2, 10 * (value_ - 1)) + start_;
	}

	/* !
	@function  EaseOutExpo

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards exponentially.

	*//*__________________________________________________________________________*/
	float EaseOutExpo(float start_, float end_, float value_)
	{
		end_ -= start_;
		return end_ * (-powf(2, -10 * value_) + 1) + start_;
	}

	/* !
	@function  EaseInOutExpo

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards exponentially.

	*//*__________________________________________________________________________*/
	float EaseInOutExpo(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return end_ * 0.5f * powf(2, 10 * (value_ - 1)) + start_;
		value_--;
		return end_ * 0.5f * (-powf(2, -10 * value_) + 2) + start_;
	}

	/* !
	@function  EaseInCirc

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards circularly.

	*//*__________________________________________________________________________*/
	float EaseInCirc(float start_, float end_, float value_)
	{
		end_ -= start_;
		return -end_ * (sqrtf(1 - value_ * value_) - 1) + start_;
	}

	/* !
	@function  EaseOutCirc

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards circularly.

	*//*__________________________________________________________________________*/
	float EaseOutCirc(float start_, float end_, float value_)
	{
		value_--;
		end_ -= start_;
		return end_ * sqrtf(1 - value_ * value_) + start_;
	}

	/* !
	@function  EaseInOutCirc

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards circularly.

	*//*__________________________________________________________________________*/
	float EaseInOutCirc(float start_, float end_, float value_)
	{
		value_ /= .5f;
		end_ -= start_;
		if (value_ < 1) return -end_ * 0.5f * (sqrtf(1 - value_ * value_) - 1) + start_;
		value_ -= 2;
		return end_ * 0.5f * (sqrtf(1 - value_ * value_) + 1) + start_;
	}

	/* !
	@function  EaseOutBounce

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards as a bounce.

	*//*__________________________________________________________________________*/
	float EaseOutBounce(float start_, float end_, float value_)
	{
		value_ /= 1.f;
		end_ -= start_;
		if (value_ < (1 / 2.75f))
		{
			return end_ * (7.5625f * value_ * value_) + start_;
		}
		else if (value_ < (2 / 2.75f))
		{
			value_ -= (1.5f / 2.75f);
			return end_ * (7.5625f * (value_)*value_ + .75f) + start_;
		}
		else if (value_ < (2.5 / 2.75))
		{
			value_ -= (2.25f / 2.75f);
			return end_ * (7.5625f * (value_)*value_ + .9375f) + start_;
		}
		else
		{
			value_ -= (2.625f / 2.75f);
			return end_ * (7.5625f * (value_)*value_ + .984375f) + start_;
		}
	}

	/* !
	@function  EaseInBounce

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards as a bounce.

	*//*__________________________________________________________________________*/
	float EaseInBounce(float start_, float end_, float value_)
	{
		end_ -= start_;
		float d = 1.f;
		return end_ - EaseOutBounce(0, end_, d - value_) + start_;
	}

	/* !
	@function  EaseInOutBounce

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards as a bounce.

	*//*__________________________________________________________________________*/
	float EaseInOutBounce(float start_, float end_, float value_)
	{
		end_ -= start_;
		float d = 1.f;
		if (value_ < d * 0.5f) return EaseInBounce(0, end_, value_ * 2) * 0.5f + start_;
		else return EaseOutBounce(0, end_, value_ * 2 - d) * 0.5f + end_ * 0.5f + start_;
	}

	/* !
	@function  EaseInBack

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards with a back.

	*//*__________________________________________________________________________*/
	float EaseInBack(float start_, float end_, float value_)
	{
		end_ -= start_;
		value_ /= 1;
		float s = 1.70158f;
		return end_ * (value_)*value_ * ((s + 1) * value_ - s) + start_;
	}

	/* !
	@function  EaseOutBack

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards with a back.

	*//*__________________________________________________________________________*/
	float EaseOutBack(float start_, float end_, float value_)
	{
		float s = 1.70158f;
		end_ -= start_;
		value_ = (value_)-1;
		return end_ * ((value_)*value_ * ((s + 1) * value_ + s) + 1) + start_;
	}

	/* !
	@function  EaseInOutBack

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards with a back.

	*//*__________________________________________________________________________*/
	float EaseInOutBack(float start_, float end_, float value_)
	{
		float s = 1.70158f;
		end_ -= start_;
		value_ /= .5f;
		if ((value_) < 1)
		{
			s *= (1.525f);
			return end_ * 0.5f * (value_ * value_ * (((s)+1) * value_ - s)) + start_;
		}
		value_ -= 2;
		s *= (1.525f);
		return end_ * 0.5f * ((value_)*value_ * (((s)+1) * value_ + s) + 2) + start_;
	}

	/* !
	@function  EaseInElastic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards elastically.

	*//*__________________________________________________________________________*/
	float EaseInElastic(float start_, float end_, float value_)
	{
		end_ -= start_;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value_ == 0) return start_;

		if ((value_ /= d) == 1) return start_ + end_;

		if (a == 0.f || a < fabs(end_))
		{
			a = end_;
			s = p / 4;
		}
		else
		{
			s = p / (PIx2) * asinf(end_ / a);
		}

		return -(a * powf(2, 10 * (value_ -= 1)) * sinf((value_ * d - s) * (PIx2) / p)) + start_;
	}

	/* !
	@function  EaseOutElastic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			outwards elastically.

	*//*__________________________________________________________________________*/
	float EaseOutElastic(float start_, float end_, float value_)
	{
		end_ -= start_;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value_ == 0) return start_;

		if ((value_ /= d) == 1) return start_ + end_;

		if (a == 0.f || a < fabs(end_))
		{
			a = end_;
			s = p * 0.25f;
		}
		else
		{
			s = p / (PIx2) * asinf(end_ / a);
		}

		return (a * powf(2, -10 * value_) * sinf((value_ * d - s) * (PIx2) / p) + end_ + start_);
	}

	/* !
	@function  EaseInOutElastic

	@params	start_ : the start value of the interpolation
	@params	end_ : the end value of the interpolation
	@params	value_ : the value to interpolate

	@brief	Interpolates between `start_` and `end_` with the given `value_`
			inwards and outwards elastically.

	*//*__________________________________________________________________________*/
	float EaseInOutElastic(float start_, float end_, float value_)
	{
		end_ -= start_;

		float d = 1.f;
		float p = d * .3f;
		float s;
		float a = 0;

		if (value_ == 0) return start_;

		if ((value_ /= d * 0.5f) == 2) return start_ + end_;

		if (a == 0.f || a < fabs(end_))
		{
			a = end_;
			s = p / 4;
		}
		else
		{
			s = p / (PIx2) * asinf(end_ / a);
		}

		if (value_ < 1) return -0.5f * (a * powf(2, 10 * (value_ -= 1)) * sinf((value_ * d - s) * (PIx2) / p)) + start_;
		return a * powf(2, -10 * (value_ -= 1)) * sinf((value_ * d - s) * (PIx2) / p) * 0.5f + end_ + start_;
	}
}
