#pragma once
#include <random>

// static struct or namespace-like ... hmm...
struct Random
{
	using state_type = long long;

	// this is temporary!!! unless?
	using vec2_type = struct { float x, y; };
	using vec3_type = struct { float x, y, z; };

	static void SetState(state_type state_); // kinda like InitState in Unity
	static state_type GetState();

	// not sure if scripts will support function overloading, so here we are
	static float RangeFloat(float minInclusive, float maxInclusive);
	static int RangeInt(int minInclusive, int maxExclusive);

	// Color ColorHSV();

	static float Value01();

	// quaternion?? Rotation();

	static vec2_type InsideUnitCircle();
	static vec2_type OnUnitCircle();
	static vec3_type InsideUnitSphere();
	static vec3_type OnUnitSphere();

	static state_type state;
	// ============================================================
	static bool Initialize();

	static bool isInitialized;
	static std::minstd_rand randomEngine;
};
