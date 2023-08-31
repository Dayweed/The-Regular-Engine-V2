#include "Vector2.h"
#include <cmath>
#include "Mathf.h"

Vector2::Vector2(float _x, float _y) : x{ _x }, y{ _y } {}

Vector2::Vector2(const Vector2& temp)
{
	x = temp.x;
	y = temp.y;
}

Vector2& Vector2::operator=(const Vector2& rhs)
{
	Vector2 tmp(rhs);
	Swap(tmp, *this);
	return *this;
}

Vector2& Vector2::operator+=(const Vector2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vector2& Vector2::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	return *this;
}

Vector2& Vector2::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	return *this;
}

Vector2 Vector2::operator+(const Vector2& rhs) const
{
	return { x + rhs.x , y + rhs.y };
}

Vector2 Vector2::operator-(const Vector2& rhs) const
{
	return { x - rhs.x , y - rhs.y };
}

Vector2 Vector2::operator*(const float rhs) const
{
	return { x * rhs, y * rhs };
}

Vector2 Vector2::operator/(const float rhs) const
{
	return { x / rhs, y / rhs };
}

bool Vector2::operator==(const Vector2& rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

bool Vector2::operator!=(const Vector2& rhs) const
{
	return !(*this == rhs);
}

Vector2 Vector2::operator-() const
{
	return { -x, -y };
}

float Vector2::Length() const
{
	return sqrtf(x * x + y * y);
}

Vector2 Vector2::Norm() const
{
	return *this / Length();
}

void Vector2::Normalize()
{
	*this /= Length();
}

Vector2 Vector2::Zero()
{
	return { 0, 0 };
}

Vector2 Vector2::Up()
{
	return { 0, 1 };
}

Vector2 Vector2::Down()
{
	return { 0, -1 };
}

Vector2 Vector2::Left()
{
	return { -1, 0 };
}

Vector2 Vector2::Right()
{
	return { 1, 0 };
}

Vector2 Vector2::One()
{
	return { 1, 1 };
}

float operator*(const Vector2& vec0, const Vector2& vec1)
{
	return vec0.x * vec1.x + vec0.y * vec1.y;
}

float operator^(const Vector2& vec0, const Vector2& vec1)
{
	return vec0.x * vec1.y - vec0.y * vec1.x;
}

void Vector2::Swap(Vector2& lhs, Vector2& rhs)
{
	Vector2 temp(lhs);
	lhs = rhs;
	rhs = temp;
}

Vector2 operator*(const float lhs, const Vector2& rhs)
{
	return { lhs * rhs.x , lhs * rhs.y };
}

float Vector2::Distance(const Vector2& ptHead, const Vector2& ptTail)
{
	return (ptHead - ptTail).Length();
}

float Vector2::Angle(const Vector2& from, const Vector2& to)
{
	// a.b = |a||b|cos(theta)
	return acos(from * to / from.Length() / to.Length()) / 3.141592653f * 180.0f;
}

Vector2 Vector2::ClampMagnitude(const Vector2& vector, float maxLength)
{
	return vector.Length() > maxLength ? vector.Norm() * maxLength : vector;
}

float Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
{
	return lhs * rhs;
}

Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t)
{
	t = Mathf::Clamp01(t);
	// a + t * (b - a)
	return a * (1 - t) + b * t;
}

Vector2 Vector2::LerpUnclamped(const Vector2& a, const Vector2& b, float t)
{
	return a * (1 - t) + b * t;
}

Vector2 Vector2::Max(const Vector2& lhs, const Vector2& rhs)
{
	return { lhs.x > rhs.x ? lhs.x : rhs.x, lhs.y > rhs.y ? lhs.y : rhs.y };
}

Vector2 Vector2::Min(const Vector2& lhs, const Vector2& rhs)
{
	return { lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y };
}

Vector2 Vector2::Perpendicular(const Vector2& inDirection)
{
	return { -inDirection.y, inDirection.x };
}

Vector2 Vector2::Reflect(const Vector2& inDirection, const Vector2& inNormal)
{
	// v' = v - 2(v.n)n, where n is a unit vector
	return inDirection - 2 * (inDirection * inNormal.Norm()) * inNormal.Norm();
}

Vector2 Vector2::Scale(const Vector2& a, const Vector2& b)
{
	return { a.x * b.x, a.y * b.y };
}

void Vector2::Scale(const Vector2& scale)
{
	x *= scale.x; y *= scale.y;
}
