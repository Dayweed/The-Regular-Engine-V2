#include "Vector2Int.h"
#include <cmath>

Vector2Int::Vector2Int(int _x, int _y) : x{ _x }, y{ _y } {}

// Vector2Int::Vector2Int(const Vector2Int& temp) : x{ temp.x }, y{ temp.y } {}

//Vector2Int& Vector2Int::operator=(const Vector2Int& rhs)
//{
//	Vector2Int tmp(rhs);
//	Swap(tmp, *this);
//	return *this;
//}

Vector2Int& Vector2Int::operator+=(const Vector2Int& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vector2Int& Vector2Int::operator-=(const Vector2Int& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vector2Int& Vector2Int::operator*=(const int rhs)
{
	x *= rhs;
	y *= rhs;
	return *this;
}

Vector2Int& Vector2Int::operator/=(const int rhs)
{
	x /= rhs;
	y /= rhs;
	return *this;
}

Vector2Int Vector2Int::operator+(const Vector2Int& rhs) const
{
	return { x + rhs.x , y + rhs.y };
}

Vector2Int Vector2Int::operator-(const Vector2Int& rhs) const
{
	return { x - rhs.x , y - rhs.y };
}

Vector2Int Vector2Int::operator*(const int rhs) const
{
	return { x * rhs, y * rhs };
}

Vector2Int Vector2Int::operator/(const int rhs) const
{
	return { x / rhs, y / rhs };
}

bool Vector2Int::operator==(const Vector2Int& rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

bool Vector2Int::operator!=(const Vector2Int& rhs) const
{
	return !(*this == rhs);
}

int Vector2Int::operator*(const Vector2Int& vec) const
{
	return x * vec.x + y * vec.y;
}

int Vector2Int::operator^(const Vector2Int& vec) const
{
	return x * vec.y - y * vec.x;
}

Vector2Int Vector2Int::operator-() const
{
	return { -x, -y };
}

float Vector2Int::Length() const
{
	return sqrtf(static_cast<float>(x * x + y * y));
}

bool Vector2Int::IsZero() const
{
	return *this == Zero();
}

Vector2Int Vector2Int::Zero()
{
	return { 0, 0 };
}

Vector2Int Vector2Int::Up()
{
	return { 0, 1 };
}

Vector2Int Vector2Int::Down()
{
	return { 0, -1 };
}

Vector2Int Vector2Int::Left()
{
	return { -1, 0 };
}

Vector2Int Vector2Int::Right()
{
	return { 1, 0 };
}

Vector2Int Vector2Int::One()
{
	return { 1, 1 };
}

void Vector2Int::Swap(Vector2Int& lhs, Vector2Int& rhs)
{
	Vector2Int temp(lhs);
	lhs = rhs;
	rhs = temp;
}

Vector2Int operator*(const int lhs, const Vector2Int& rhs)
{
	return { lhs * rhs.x , lhs * rhs.y };
}

float Vector2Int::Distance(const Vector2Int& ptHead, const Vector2Int& ptTail)
{
	return (ptHead - ptTail).Length();
}

Vector2Int Vector2Int::Max(const Vector2Int& lhs, const Vector2Int& rhs)
{
	return { lhs.x > rhs.x ? lhs.x : rhs.x, lhs.y > rhs.y ? lhs.y : rhs.y };
}

Vector2Int Vector2Int::Min(const Vector2Int& lhs, const Vector2Int& rhs)
{
	return { lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y };
}

Vector2Int Vector2Int::Perpendicular(const Vector2Int& inDirection)
{
	return { -inDirection.y, inDirection.x };
}

Vector2Int Vector2Int::Scale(const Vector2Int& a, const Vector2Int& b)
{
	return { a.x * b.x, a.y * b.y };
}

void Vector2Int::Scale(const Vector2Int& scale)
{
	x *= scale.x; y *= scale.y;
}

Vector2Int Vector2Int::CeilToInt(const Vector2& v)
{
	return { static_cast<int>(ceilf(v.x)), static_cast<int>(ceilf(v.y)) };
}

Vector2Int Vector2Int::FloorToInt(const Vector2& v)
{
	return { static_cast<int>(v.x), static_cast<int>(v.y) };
}
