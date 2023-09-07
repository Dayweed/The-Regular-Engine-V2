/*!
	@file      Random.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains a number of declarations of commonly used
			   random functions, similar to Unity's Random functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include <random>
#include "Vector2.h"
#include "Vector3.h"

// static struct or namespace-like ... hmm...
struct Random
{
	using state_type = long long;

	static void SetState(state_type state_); // kinda like InitState in Unity

	static state_type GetState();

	// not sure if scripts will support function overloading, so here we are

	/* !
	@function      RangeFloat
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        minInclusive    The minimum value of the range (inclusive).
	@params        maxInclusive    The maximum value of the range (inclusive).

	@brief         Returns a random float within [minInclusive..maxInclusive]
				   (range is inclusive).
	*//*__________________________________________________________________________*/
	static float RangeFloat(float minInclusive, float maxInclusive);

	/* !
	@function      RangeInt
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@params        minInclusive    The minimum value of the range (inclusive).
	@params        maxExclusive    The maximum value of the range (exclusive).

	@brief         Return a random int within [minInclusive..maxExclusive).
	*//*__________________________________________________________________________*/
	static int RangeInt(int minInclusive, int maxExclusive);

	// Color ColorHSV();

	/* !
	@function      Value01
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         Returns a random float within [0.0..1.0] (range is inclusive).
	*//*__________________________________________________________________________*/
	static float Value01();

	// quaternion?? Rotation();

	/* !
	@function      InsideUnitCircle
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         Returns a random point inside or on a circle with radius 1.0.
	*//*__________________________________________________________________________*/
	static Vector2 InsideUnitCircle();

	/* !
	@function      OnUnitCircle
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         Returns a random point on the circumference of a circle with
				   radius 1.0.
	*//*__________________________________________________________________________*/
	static Vector2 OnUnitCircle();

	/* !
	@function      InsideUnitSphere
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         Returns a random point inside or on a sphere with radius 1.0.
	*//*__________________________________________________________________________*/
	static Vector3 InsideUnitSphere();

	/* !
	@function      OnUnitSphere
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         Returns a random point on the surface of a sphere with radius 1.0.
	*//*__________________________________________________________________________*/
	static Vector3 OnUnitSphere();

	static state_type state;

private:

	/* !
	@function      Initialize
	@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

	@brief         This function sets the random engine's internal seed and state.
	*//*__________________________________________________________________________*/
	static bool Initialize();

	static bool isInitialized;
	static std::minstd_rand randomEngine;
};
