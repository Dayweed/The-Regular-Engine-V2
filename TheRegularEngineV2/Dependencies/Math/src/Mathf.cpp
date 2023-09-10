/*!
	@file      Mathf.cpp
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains a number of definitions of commonly used
			   math functions, similar to Unity's Mathf functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#include "Mathf.h"
#include "Easing.h"

#define _USE_MATH_DEFINES
#include <cmath>
// with the define, M_PI and other math constants are now defined
#include <cfloat> // for FLT_EPSILON...apparently?? because v143 needs this??

namespace Mathf
{
	float DegToRad(float deg) { return deg * static_cast<float>(M_PI) / 180.0f; }

	float RadToDeg(float rad) { return rad / static_cast<float>(M_PI) * 180.0f; }

	float Abs(float f) { return abs(f); }

	int Abs(int value) { return abs(value); }

	float Acos(float f) { return acos(f); }

	float Asin(float f) { return asin(f); }

	float Atan(float f) { return atan(f); }

	float Atan2(float y, float x) { return atan2(y, x); }

	float Ceil(float f) { return ceil(f); }

	int CeilToInt(float f) { return static_cast<int>(ceil(f)); }

	int Clamp(int value, int min, int max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	};

	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	};

	float Clamp01(float value)
	{
		if (value < 0) return 0;
		if (value > 1) return 1;
		return value;
	}

	float Cos(float f) { return cos(f); }

	float Sin(float f) { return sin(f); }

	float Tan(float f) { return tan(f); }

	float Exp(float power) { return powf(static_cast<float>(M_E), power); }

	float Floor(float f) { return floor(f); }

	int FloorToInt(float f) { return static_cast<int>(floor(f)); }

	float InverseLerp(float a, float b, float value)
	{
		// ensure that a and b are different
		return abs(a - b) < FLT_EPSILON ? value : (value - a) / (b - a);
	}

	float Lerp(float a, float b, float t) { return Clamp(Easing::EaseLinear(a, b, t), a, b); }

	float LerpUnclamped(float a, float b, float t) { return Easing::EaseLinear(a, b, t); }

	float Log(float f, float p) { return log(f) / log(p); } // logarithms - change of base rule

	float Log(float f) { return log(f); }

	float Log10(float f) { return log10(f); }

	int Max(int a, int b) { return a > b ? a : b; }

	float Max(float a, float b) { return a > b ? a : b; }

	int Min(int a, int b) { return a < b ? a : b; }

	float Min(float a, float b) { return a < b ? a : b; }

	// float PerlinNoise(float x, float y); // normalised

	float Pow(float f, float p) { return pow(f, p); }

	float Repeat(float t, float length) { return fmod(t, length); }

	float Round(float f) { return round(f); }

	int RoundToInt(float f) { return static_cast<int>(round(f)); }

	float Sign(float f) { return f >= 0 ? 1.0f : -1.0f; }

	float SmoothStep(float from, float to, float t)
	{
		t = Clamp01((t - from) / (to - from));
		return t * t * (3.0f - 2.0f * t);
	}

	float Sqrt(float f) { return sqrt(f); }
}
