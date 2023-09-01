#include "Vector3Int.h"
#include <cmath>

Vector3Int::Vector3Int(int _x, int _y, int _z) : x{ _x }, y{ _y }, z{ _z } {}

// Vector3Int::Vector3Int(const Vector3Int& temp) : x{ temp.x }, y{ temp.y }, z{ temp.z } {}

//Vector3Int& Vector3Int::operator=(const Vector3Int& rhs)
//{
//	Vector3Int tmp(rhs);
//	Swap(tmp, *this);
//	return *this;
//}

Vector3Int& Vector3Int::operator+=(const Vector3Int& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

Vector3Int& Vector3Int::operator-=(const Vector3Int& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Vector3Int& Vector3Int::operator*=(const int rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

Vector3Int& Vector3Int::operator/=(const int rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

Vector3Int Vector3Int::operator+(const Vector3Int& rhs) const
{
	return { x + rhs.x , y + rhs.y, z + rhs.z };
}

Vector3Int Vector3Int::operator-(const Vector3Int& rhs) const
{
	return { x - rhs.x , y - rhs.y, z - rhs.z };
}

Vector3Int Vector3Int::operator*(const int rhs) const
{
	return { x * rhs, y * rhs, z * rhs };
}

Vector3Int Vector3Int::operator/(const int rhs) const
{
	return { x / rhs, y / rhs, z / rhs };
}

bool Vector3Int::operator==(const Vector3Int& rhs) const
{
	return (x == rhs.x && y == rhs.y && z == rhs.z);
}

bool Vector3Int::operator!=(const Vector3Int& rhs) const
{
	return !(*this == rhs);
}

int Vector3Int::operator*(const Vector3Int& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

Vector3Int Vector3Int::operator^(const Vector3Int& vec) const
{
	return { y * vec.z - z * vec.y,
		z * vec.x - x * vec.z, x * vec.y - y * vec.x };
}

Vector3Int Vector3Int::operator-() const
{
	return { -x, -y, -z };
}

float Vector3Int::Length() const
{
	return sqrtf(static_cast<float>(x * x + y * y + z * z));
}

bool Vector3Int::IsZero() const
{
	return *this == Zero();
}

Vector3Int Vector3Int::Zero()
{
	return { 0, 0, 0 };
}

Vector3Int Vector3Int::Up()
{
	return { 0, 1, 0 };
}

Vector3Int Vector3Int::Down()
{
	return { 0, -1, 0 };
}

Vector3Int Vector3Int::Left()
{
	return { -1, 0, 0 };
}

Vector3Int Vector3Int::Right()
{
	return { 1, 0, 0 };
}

inline Vector3 Vector3Int::Forward()
{
	return { 0, 0, 1 };
}

inline Vector3 Vector3Int::Back()
{
	return { 0, 0, -1 };
}

Vector3Int Vector3Int::One()
{
	return { 1, 1, 1 };
}

void Vector3Int::Swap(Vector3Int& lhs, Vector3Int& rhs)
{
	Vector3Int temp(lhs);
	lhs = rhs;
	rhs = temp;
}

Vector3Int operator*(const int lhs, const Vector3Int& rhs)
{
	return { lhs * rhs.x , lhs * rhs.y, lhs * rhs.z };
}

float Vector3Int::Distance(const Vector3Int& ptHead, const Vector3Int& ptTail)
{
	return (ptHead - ptTail).Length();
}

Vector3Int Vector3Int::Max(const Vector3Int& lhs, const Vector3Int& rhs)
{
	return { lhs.x > rhs.x ? lhs.x : rhs.x,
		lhs.y > rhs.y ? lhs.y : rhs.y, lhs.z > rhs.z ? lhs.z : rhs.z };
}

Vector3Int Vector3Int::Min(const Vector3Int& lhs, const Vector3Int& rhs)
{
	return { lhs.x < rhs.x ? lhs.x : rhs.x,
		lhs.y < rhs.y ? lhs.y : rhs.y, lhs.z < rhs.z ? lhs.z : rhs.z };
}

Vector3Int Vector3Int::Scale(const Vector3Int& a, const Vector3Int& b)
{
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

void Vector3Int::Scale(const Vector3Int& scale)
{
	x *= scale.x; y *= scale.y; z *= scale.z;
}

Vector3Int Vector3Int::CeilToInt(const Vector3& v)
{
	return { static_cast<int>(ceilf(v.x)),
		static_cast<int>(ceilf(v.y)), static_cast<int>(ceilf(v.z)) };
}

Vector3Int Vector3Int::FloorToInt(const Vector3& v)
{
	return { static_cast<int>(v.x), static_cast<int>(v.y), static_cast<int>(v.z) };
}
