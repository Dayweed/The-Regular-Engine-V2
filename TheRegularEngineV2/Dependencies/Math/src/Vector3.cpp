/*!
	@file      Vector3.cpp
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains the definitions of the Vector3 member
			   functions, which is a useful representation of a 3D vector.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#include "Vector3.h"
#include <cmath>
#include "Mathf.h"

#define EPSILON (0.00001f)

Vector3::Vector3(const float scalar) : x{ scalar }, y{ scalar }, z{ scalar } {}

Vector3::Vector3(const float _x, const float _y, const float _z) : x{ _x }, y{ _y }, z{ _z } {}

Vector3::Vector3(const glm::vec3 v) : x{ v.x }, y{ v.y }, z{ v.z } {}

Vector3& Vector3::operator+=(const Vector3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Vector3& Vector3::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

Vector3& Vector3::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return { x + rhs.x , y + rhs.y, z + rhs.z };
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return { x - rhs.x , y - rhs.y, z - rhs.z };
}

Vector3 Vector3::operator*(const float rhs) const
{
	return { x * rhs, y * rhs, z * rhs };
}

Vector3 Vector3::operator/(const float rhs) const
{
	return { x / rhs, y / rhs, z / rhs };
}

bool Vector3::operator==(const Vector3& rhs) const
{
	return fabs(x - rhs.x) < EPSILON &&
		fabs(y - rhs.y) < EPSILON &&
		fabs(z - rhs.z) < EPSILON;
}

bool Vector3::operator!=(const Vector3& rhs) const
{
	return !(*this == rhs);
}

float Vector3::operator*(const Vector3& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

Vector3 Vector3::operator^(const Vector3& vec) const
{
	return { y * vec.z - z * vec.y,
		z * vec.x - x * vec.z, x * vec.y - y * vec.x };
}

Vector3::operator glm::vec3() const
{
	return { x, y, z };
}

Vector3 Vector3::operator-() const
{
	return { -x, -y, -z };
}

float Vector3::Length() const
{
	return sqrtf(x * x + y * y + z * z);
}

bool Vector3::IsZero() const
{
	return *this == Zero();
}

Vector3 Vector3::Norm() const
{
	return IsZero() ? *this : *this / Length();
}

void Vector3::Normalize()
{
	*this /= IsZero() ? 1 : Length();
}

Vector3 Vector3::Zero()
{
	return { 0, 0, 0 };
}

Vector3 Vector3::Up()
{
	return { 0, 1, 0 };
}

Vector3 Vector3::Down()
{
	return { 0, -1, 0 };
}

Vector3 Vector3::Left()
{
	return { -1, 0, 0 };
}

Vector3 Vector3::Right()
{
	return { 1, 0, 0 };
}

Vector3 Vector3::Forward()
{
	return { 0, 0, 1 };
}

Vector3 Vector3::Back()
{
	return { 0, 0, -1 };
}

Vector3 Vector3::One()
{
	return { 1, 1, 1 };
}

void Vector3::Swap(Vector3& lhs, Vector3& rhs)
{
	Vector3 temp(lhs);
	lhs = rhs;
	rhs = temp;
}

float Vector3::Distance(const Vector3& ptHead, const Vector3& ptTail)
{
	return (ptHead - ptTail).Length();
}

float Vector3::Angle(const Vector3& from, const Vector3& to)
{
	// a.b = |a||b|cos(theta)
	return acos(from * to / from.Length() / to.Length()) / 3.141592653f * 180.0f;
}

Vector3 Vector3::ClampMagnitude(const Vector3& vector, float maxLength)
{
	return vector.Length() > maxLength ? vector.Norm() * maxLength : vector;
}

Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
{
	return lhs ^ rhs;
}

float Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
{
	return lhs * rhs;
}

Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t)
{
	t = Mathf::Clamp01(t);
	// a + t * (b - a)
	return a * (1 - t) + b * t;
}

Vector3 Vector3::LerpUnclamped(const Vector3& a, const Vector3& b, float t)
{
	return a * (1 - t) + b * t;
}

Vector3 Vector3::Max(const Vector3& lhs, const Vector3& rhs)
{
	return { lhs.x > rhs.x ? lhs.x : rhs.x,
		lhs.y > rhs.y ? lhs.y : rhs.y, lhs.z > rhs.z ? lhs.z : rhs.z };
}

Vector3 Vector3::Min(const Vector3& lhs, const Vector3& rhs)
{
	return { lhs.x < rhs.x ? lhs.x : rhs.x,
		lhs.y < rhs.y ? lhs.y : rhs.y, lhs.z < rhs.z ? lhs.z : rhs.z };
}

Vector3 Vector3::Project(const Vector3& vector, const Vector3& onNormal)
{
	return vector * onNormal.Norm() * onNormal.Norm();
}

Vector3 Vector3::Reflect(const Vector3& inDirection, const Vector3& inNormal)
{
	// v' = v - 2(v.n)n, where n is a unit vector
	// result = -v'
	return -(inDirection - 2 * (inDirection * inNormal.Norm()) * inNormal.Norm());
}

Vector3 Vector3::Scale(const Vector3& a, const Vector3& b)
{
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

void Vector3::Scale(const Vector3& scale)
{
	x *= scale.x; y *= scale.y; z *= scale.z;
}

Vector3 Vector3::Slerp(const Vector3& a, const Vector3& b, float t)
{
	return SlerpUnclamped(a, b, Mathf::Clamp01(t));
}

// thank you https://en.wikipedia.org/wiki/Slerp#Geometric_Slerp
Vector3 Vector3::SlerpUnclamped(const Vector3& a, const Vector3& b, float t)
{
	const float rad = Mathf::DegToRad(Angle(a, b));
	const Vector3 unitVec = 1 / sinf(rad) * (sinf((1 - t) * rad) * a.Norm() + sinf(t * rad) * b.Norm());
	return unitVec.Norm() * Mathf::LerpUnclamped(a.Length(), b.Length(), t);
}

Vector3 operator*(const float lhs, const Vector3& rhs)
{
	return { lhs * rhs.x , lhs * rhs.y, lhs * rhs.z };
}

Vector3 operator/(const float lhs, const Vector3& rhs)
{
	return { lhs / rhs.x , lhs / rhs.y, lhs / rhs.z };
}
