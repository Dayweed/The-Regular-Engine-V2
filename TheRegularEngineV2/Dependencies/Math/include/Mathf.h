/*!
	@file      Mathf.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains a number of declarations of commonly used
			   math functions, similar to Unity's Mathf functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once

// NEVER EVER CALL A FILE 'MATH.H', NO MATTER UPPER/LOWER CASE.
// WINDOWS WILL PRETEND IT'S THE OFFICIAL 'math.h' AND RUIN YOUR LIFE.
// (unless you HAPPEN to define ALL the things that are in the official library)
// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

// for constexpr functions, DEFINE it in a .h because preprocessor stuff
namespace Mathf
{
	/* !
	@function      DegToRad
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        deg    The angle in degrees to be converted to radians.

	@brief         Returns the given angle in radians.
	*//*__________________________________________________________________________*/
	float DegToRad(float deg);

	/* !
	@function      RadToDeg
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        rad    The angle in radians to be converted to degrees.

	@brief         Returns the given angle in degrees.
	*//*__________________________________________________________________________*/
	float RadToDeg(float rad);

	// because the PI macro doesn't work apparently?!
	inline float pi = 3.141592653f;

	/* !
	@function      Abs
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the absolute value of.

	@brief         Returns the absolute value of the given float.
	*//*__________________________________________________________________________*/
	float Abs(float f);

	/* !
	@function      Abs
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        value    The value to return the absolute value of.

	@brief         Returns the absolute value of the given value.
	*//*__________________________________________________________________________*/
	int Abs(int value);

	/* !
	@function      Acos
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the arc-cosine of.

	@brief         Returns the arc-cosine of f - the angle in radians whose cosine
				   is f.
	*//*__________________________________________________________________________*/
	float Acos(float f);

	/* !
	@function      Asin
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the arc-sine of.

	@brief         Returns the arc-sine of f - the angle in radians whose sine is f.
	*//*__________________________________________________________________________*/
	float Asin(float f);

	/* !
	@function      Atan
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the arc-tangent of.

	@brief         Returns the arc-tangent of f - the angle in radians whose
				   tangent is f.
	*//*__________________________________________________________________________*/
	float Atan(float f);

	/* !
	@function      Atan2
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        y    The vertical component of the angle.
	@params        x    The horizontal component of the angle.

	@brief         Returns the angle in radians whose Tan is y/x.
	*//*__________________________________________________________________________*/
	float Atan2(float y, float x);

	/* !
	@function      Ceil
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the ceiling of.

	@brief         Returns the smallest whole number greater to or equal to f.
	*//*__________________________________________________________________________*/
	float Ceil(float f);

	/* !
	@function      CeilToInt
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the ceiling of as an integer.

	@brief         Returns the smallest integer greater to or equal to f.
	*//*__________________________________________________________________________*/
	int CeilToInt(float f);

	/* !
	@function      Clamp
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        value    The integer point value to restrict inside the
							min-to-max range.
	@params        min      The minimum integer point value to compare against.
	@params        max      The maximum integer point value to compare against.

	@brief         Clamps the given value between a range defined by the given
				   minimum integer and maximum integer values. Returns the given
				   value if it is within min and max.
	*//*__________________________________________________________________________*/
	int Clamp(int value, int min, int max);

	/* !
	@function      Clamp
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        value    The floating point value to restrict inside the range
							defined by the minimum and maximum values.
	@params        min      The minimum floating point value to compare against.
	@params        max      The maximum floating point value to compare against.

	@brief         Clamps the given value between the given minimum float and
				   maximum float values. Returns the given value if it is within
				   the minimum and maximum range.
	*//*__________________________________________________________________________*/
	float Clamp(float value, float min, float max);

	/* !
	@function      Clamp01
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        value    The floating point value to restrict inside the range
							[0, 1].

	@brief         Clamps the given value between 0 and 1. Returns the given value
				   if it is within the range [0, 1].
	*//*__________________________________________________________________________*/
	float Clamp01(float value);

	/* !
	@function      Cos
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The input angle, in radians.

	@brief         Returns the cosine of angle f in radians.
	*//*__________________________________________________________________________*/
	float Cos(float f);

	/* !
	@function      Sin
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The input angle, in radians.

	@brief         Returns the sine of angle f in radians.
	*//*__________________________________________________________________________*/
	float Sin(float f);

	/* !
	@function      Tan
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The input angle, in radians.

	@brief         Returns the tangent of angle f in radians.
	*//*__________________________________________________________________________*/
	float Tan(float f);

	/* !
	@function      Exp
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        power    The power to raise e to.

	@brief         Returns e raised to the specified power.
	*//*__________________________________________________________________________*/
	float Exp(float power);

	/* !
	@function      Floor
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the floor of.

	@brief         Returns the greatest whole number lesser than to or equal to f.
	*//*__________________________________________________________________________*/
	float Floor(float f);

	/* !
	@function      FloorToInt
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to return the floor of as an integer.

	@brief         Returns the greatest integer lesser than to or equal to f.
	*//*__________________________________________________________________________*/
	int FloorToInt(float f);

	/* !
	@function      InverseLerp
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        a        The start of the range.
	@params        b        The end of the range.
	@params        value    The point within the range you want to calculate.

	@brief         Determines where a value lies between two points.
	*//*__________________________________________________________________________*/
	float InverseLerp(float a, float b, float value);

	/* !
	@function      Lerp
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        a    The start value.
	@params        b    The end value.
	@params        t    The interpolation value between the two floats.

	@brief         Linearly interpolates between a and b by t.
	*//*__________________________________________________________________________*/
	float Lerp(float a, float b, float t);

	/* !
	@function      LerpUnclamped
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        a    The start value.
	@params        b    The end value.
	@params        t    The interpolation value between the two floats.

	@brief         Linearly interpolates between a and b by t with no limit to t.
	*//*__________________________________________________________________________*/
	float LerpUnclamped(float a, float b, float t);

	/* !
	@function      Log
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to take the logarithm of.
	@params        p    The base of the logarithm.

	@brief         Returns the logarithm of a specified number in a specified base.
	*//*__________________________________________________________________________*/
	float Log(float f, float p);

	/* !
	@function      Log
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to take the logarithm of.

	@brief         Returns the natural (base e) logarithm of a specified number.
	*//*__________________________________________________________________________*/
	float Log(float f);

	/* !
	@function      Log10
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to take the logarithm of.

	@brief         Returns the base 10 logarithm of a specified number.
	*//*__________________________________________________________________________*/
	float Log10(float f);

	/* !
	@function      Max
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The first value.
	@params        p    The second value.

	@brief         Returns largest of two or more values.
	*//*__________________________________________________________________________*/
	int Max(int a, int b);

	/* !
	@function      Max
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The first value.
	@params        p    The second value.

	@brief         Returns largest of two or more values.
	*//*__________________________________________________________________________*/
	float Max(float a, float b);

	/* !
	@function      Min
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The first value.
	@params        p    The second value.

	@brief         Returns smallest of two or more values.
	*//*__________________________________________________________________________*/
	int Min(int a, int b);

	/* !
	@function      Min
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The first value.
	@params        p    The second value.

	@brief         Returns smallest of two or more values.
	*//*__________________________________________________________________________*/
	float Min(float a, float b);

	// float PerlinNoise(float x, float y); // normalized

	/* !
	@function      Pow
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to be used as the base.
	@params        p    The power to raise the value to.

	@brief         Returns f raised to power p.
	*//*__________________________________________________________________________*/
	float Pow(float f, float p);

	/* !
	@function      Repeat
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        t         The value to be repeated/wrapped.
	@params        length    The maximum length without repetition beginning.

	@brief         Loops the value t, so that it is never larger than length and
				   never smaller than 0.
	*//*__________________________________________________________________________*/
	float Repeat(float t, float length);

	/* !
	@function      Round
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to be rounded.

	@brief         Returns f rounded to the nearest whole number.
	*//*__________________________________________________________________________*/
	float Round(float f);

	/* !
	@function      RoundToInt
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to be rounded.

	@brief         Returns f rounded to the nearest integer.
	*//*__________________________________________________________________________*/
	int RoundToInt(float f);

	/* !
	@function      Sign
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to determine the sign of.

	@brief         Return value is 1 when f is positive or zero, -1 when f is
				   negative.
	*//*__________________________________________________________________________*/
	float Sign(float f);

	/* !
	@function      SmoothStep
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        from    The minimum floating point value to compare against.
	@params        to      The maximum floating point value to compare against.
	@params        t       The interpolation value between the two floats.

	@brief         Interpolates between from and to with smoothing at the limits.
	*//*__________________________________________________________________________*/
	float SmoothStep(float from, float to, float t);

	/* !
	@function      Sqrt
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        f    The value to determine the square root of.

	@brief         Returns square root of f.
	*//*__________________________________________________________________________*/
	float Sqrt(float f);
}
