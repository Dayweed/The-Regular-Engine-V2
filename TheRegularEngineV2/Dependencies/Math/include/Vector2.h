/*!
	@file      Vector2.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains the definition of the Vector2 class,
			   which is a useful representation of a 2D vector.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "../../GLM/include/glm/glm.hpp" // for glm::vec

struct Vector2
{
	// Constructors & Destructor
	Vector2() = default;
	Vector2(const float scalar);
	Vector2(const float _x, const float _y);
	Vector2(const Vector2& temp) = default;
	Vector2(Vector2&& temp) = default;
	// Vector2(const Vector3& temp);
	~Vector2() = default;

	Vector2& operator=(const Vector2& rhs) = default;
	Vector2& operator=(Vector2&& rhs) = default;

	//Arithmetic operators
	Vector2& operator+=(const Vector2& rhs);
	Vector2& operator-=(const Vector2& rhs);
	Vector2& operator*=(const float rhs);
	Vector2& operator/=(const float rhs);

	//Binary operators
	Vector2 operator+(const Vector2& rhs) const;
	Vector2 operator-(const Vector2& rhs) const;
	Vector2 operator*(const float rhs) const;
	Vector2 operator/(const float rhs) const;
	bool operator==(const Vector2& rhs) const;
	bool operator!=(const Vector2& rhs) const;

	float operator*(const Vector2& vec) const; //Dot product
	float operator^(const Vector2& vec) const; //Cross product / 'Determinant'

	// Conversion Operators
	operator glm::vec2() const;
	// operator Vector3();

	//Unary operators
	Vector2 operator-() const;

	//Length
	float Length() const;
	bool IsZero() const;

	//Normalize
	Vector2 Norm() const;
	void Normalize();

	// Convenience Functions
	static Vector2 Zero();
	static Vector2 Up();
	static Vector2 Down();
	static Vector2 Left();
	static Vector2 Right();
	static Vector2 One();
	// (+)ve infinity, (-) infinity

	static void Swap(Vector2& lhs, Vector2& rhs);

	static float Distance(const Vector2& ptHead, const Vector2& ptTail);

	static float Angle(const Vector2& from, const Vector2& to); // in degrees

	static Vector2 ClampMagnitude(const Vector2& vector, float maxLength);

	static float Dot(const Vector2& lhs, const Vector2& rhs);

	static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);

	static Vector2 LerpUnclamped(const Vector2& a, const Vector2& b, float t);

	static Vector2 Max(const Vector2& lhs, const Vector2& rhs);

	static Vector2 Min(const Vector2& lhs, const Vector2& rhs);

	// static Vector2 MoveTowards(Vector2& current, const Vector2& target, float maxDistanceDelta);

	static Vector2 Perpendicular(const Vector2& inDirection);

	static Vector2 Reflect(const Vector2& inDirection, const Vector2& inNormal);

	static Vector2 Scale(const Vector2& a, const Vector2& b);

	void Scale(const Vector2& scale);

	// static float SignedAngle(const Vector2& from, const Vector2& to);

	// Vector2.SmoothDamp

	float x{}, y{};
};

using Vec2 = Vector2;

//Non member functions
Vector2 operator*(const float lhs, const Vector2& rhs);
Vector2 operator/(const float lhs, const Vector2& rhs);
