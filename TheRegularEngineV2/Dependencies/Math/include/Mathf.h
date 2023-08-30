#pragma once

// NEVER EVER CALL A FILE 'MATH.H', NO MATTER UPPER/LOWER CASE.
// WINDOWS WILL PRETEND IT'S THE OFFICIAL 'math.h' AND RUIN YOUR LIFE.
// (unless you HAPPEN to define ALL the things that are in the official library)
// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

// for constexpr functions, DEFINE it in a .h because preprocessor stuff
namespace Mathf
{
	float DegToRad(float deg);
	float RadToDeg(float rad);
	// because the PI macro doesn't work apparently?!
	inline float pi = 3.141592653f;

	float Abs(float f);
	int Abs(int value);

	float Acos(float f);
	float Asin(float f);
	float Atan(float f);
	float Atan2(float y, float x);

	float Ceil(float f);
	int CeilToInt(float f);

	int Clamp(int value, int min, int max);
	float Clamp(float value, float min, float max);
	float Clamp01(float value);

	float Cos(float f);
	float Sin(float f);
	float Tan(float f);

	float Exp(float power);

	float Floor(float f);
	int FloorToInt(float f);

	float InverseLerp(float a, float b, float value);
	float Lerp(float a, float b, float t);
	float LerpUnclamped(float a, float b, float t);

	float Log(float f, float p);
	float Log(float f);
	float Log10(float f);

	int Max(int a, int b);
	float Max(float a, float b);
	int Min(int a, int b);
	float Min(float a, float b);
	// float PerlinNoise(float x, float y); // normalised

	float Pow(float f, float p);

	float Round(float f);
	int RoundToInt(float f);

	float Sign(float f);

	float SmoothStep(float from, float to, float t);

	float Sqrt(float f);
}
