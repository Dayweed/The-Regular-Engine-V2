/*!
	@file      Easing.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains a number of declarations of commonly used
			   easing functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once

namespace Mathf::Easing
{
	/* !
	@function      Linear
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   linearly.
	*//*__________________________________________________________________________*/
	float EaseLinear(float start, float end, float value);

	/* !
	@function      EaseInQuad
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards quadratically.
	*//*__________________________________________________________________________*/
	float EaseInQuad(float start, float end, float value);

	/* !
	@function      EaseOutQuad
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards quadratically.
	*//*__________________________________________________________________________*/
	float EaseOutQuad(float start, float end, float value);

	/* !
	@function      EaseInOutQuad
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards quadratically.
	*//*__________________________________________________________________________*/
	float EaseInOutQuad(float start, float end, float value);

	/* !
	@function      EaseInCubic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards cubically.
	*//*__________________________________________________________________________*/
	float EaseInCubic(float start, float end, float value);

	/* !
	@function      EaseOutCubic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards cubically.
	*//*__________________________________________________________________________*/
	float EaseOutCubic(float start, float end, float value);

	/* !
	@function      EaseInOutCubic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards cubically.
	*//*__________________________________________________________________________*/
	float EaseInOutCubic(float start, float end, float value);

	/* !
	@function      EaseInQuart
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards quartically.
	*//*__________________________________________________________________________*/
	float EaseInQuart(float start, float end, float value);

	/* !
	@function      EaseOutQuart
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards quartically.
	*//*__________________________________________________________________________*/
	float EaseOutQuart(float start, float end, float value);

	/* !
	@function      EaseInOutQuart
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards quartically.
	*//*__________________________________________________________________________*/
	float EaseInOutQuart(float start, float end, float value);

	/* !
	@function      EaseInQuint
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards quintically.
	*//*__________________________________________________________________________*/
	float EaseInQuint(float start, float end, float value);

	/* !
	@function      EaseOutQuint
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards quintically.
	*//*__________________________________________________________________________*/
	float EaseOutQuint(float start, float end, float value);

	/* !
	@function      EaseInOutQuint
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards quintically.
	*//*__________________________________________________________________________*/
	float EaseInOutQuint(float start, float end, float value);

	/* !
	@function      EaseInSine
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards sinusoidally.
	*//*__________________________________________________________________________*/
	float EaseInSine(float start, float end, float value);

	/* !
	@function      EaseOutSine
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards sinusoidally.
	*//*__________________________________________________________________________*/
	float EaseOutSine(float start, float end, float value);

	/* !
	@function      EaseInOutSine
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards sinusoidally.
	*//*__________________________________________________________________________*/
	float EaseInOutSine(float start, float end, float value);

	/* !
	@function      EaseInExpo
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards exponentially.
	*//*__________________________________________________________________________*/
	float EaseInExpo(float start, float end, float value);

	/* !
	@function      EaseOutExpo
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards exponentially.
	*//*__________________________________________________________________________*/
	float EaseOutExpo(float start, float end, float value);

	/* !
	@function      EaseInOutExpo
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards exponentially.
	*//*__________________________________________________________________________*/
	float EaseInOutExpo(float start, float end, float value);

	/* !
	@function      EaseInCirc
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards circularly.
	*//*__________________________________________________________________________*/
	float EaseInCirc(float start, float end, float value);

	/* !
	@function      EaseOutCirc
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards circularly.
	*//*__________________________________________________________________________*/
	float EaseOutCirc(float start, float end, float value);

	/* !
	@function      EaseInOutCirc
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards circularly.
	*//*__________________________________________________________________________*/
	float EaseInOutCirc(float start, float end, float value);

	/* !
	@function      EaseOutBounce
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards as a bounce.
	*//*__________________________________________________________________________*/
	float EaseOutBounce(float start, float end, float value);

	/* !
	@function      EaseInBounce
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards as a bounce.
	*//*__________________________________________________________________________*/
	float EaseInBounce(float start, float end, float value);

	/* !
	@function      EaseInOutBounce
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards as a bounce.
	*//*__________________________________________________________________________*/
	float EaseInOutBounce(float start, float end, float value);

	/* !
	@function      EaseInBack
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards with a back.
	*//*__________________________________________________________________________*/
	float EaseInBack(float start, float end, float value);

	/* !
	@function      EaseOutBack
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards with a back.
	*//*__________________________________________________________________________*/
	float EaseOutBack(float start, float end, float value);

	/* !
	@function      EaseInOutBack
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards with a back.
	*//*__________________________________________________________________________*/
	float EaseInOutBack(float start, float end, float value);

	/* !
	@function      EaseInElastic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards elastically.
	*//*__________________________________________________________________________*/
	float EaseInElastic(float start, float end, float value);

	/* !
	@function      EaseOutElastic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   outwards elastically.
	*//*__________________________________________________________________________*/
	float EaseOutElastic(float start, float end, float value);

	/* !
	@function      EaseInOutElastic
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        start    the start value of the interpolation
	@params        end      the end value of the interpolation
	@params        value    the value to interpolate

	@brief         Interpolates between `start` and `end` with the given `value`
				   inwards and outwards elastically.
	*//*__________________________________________________________________________*/
	float EaseInOutElastic(float start, float end, float value);
}
