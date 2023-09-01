#pragma once
#include "Vector2.h"

struct Vector2Int
{
	//Constructors, Destructor
	Vector2Int(int _x = 0, int _y = 0);
	// Vector2Int(const Vector3Int& temp);
	// Vector2Int(const Vector2Int& temp) = default;
	// Vector2Int(Vector2Int&& temp) = default;

	// Vector2Int& operator=(const Vector2Int& rhs) = default;
	// Vector2Int& operator=(Vector2Int&& rhs) = default;

	//Arithmetic operators
	Vector2Int& operator+=(const Vector2Int& rhs);
	Vector2Int& operator-=(const Vector2Int& rhs);
	Vector2Int& operator*=(const int rhs);
	Vector2Int& operator/=(const int rhs);

	//Binary operators
	Vector2Int operator+(const Vector2Int& rhs) const;
	Vector2Int operator-(const Vector2Int& rhs) const;
	Vector2Int operator*(const int rhs) const;
	Vector2Int operator/(const int rhs) const;
	bool operator==(const Vector2Int& rhs) const;
	bool operator!=(const Vector2Int& rhs) const;

	int operator*(const Vector2Int& vec) const; //Dot product
	int operator^(const Vector2Int& vec) const; //Cross product / 'Determinant'

	// Conversion Operators
	// operator glm::vec2();
	// operator physx::PxVec2();
	// operator Vector3Int();

	//Unary operators
	Vector2Int operator-() const;

	//Length
	float Length() const;
	bool IsZero() const;

	// Convenience Functions
	static Vector2Int Zero();
	static Vector2Int Up();
	static Vector2Int Down();
	static Vector2Int Left();
	static Vector2Int Right();
	static Vector2Int One();
	// (+)ve infinity, (-) infinity

	static void Swap(Vector2Int& lhs, Vector2Int& rhs);

	static float Distance(const Vector2Int& ptHead, const Vector2Int& ptTail);

	static Vector2Int Max(const Vector2Int& lhs, const Vector2Int& rhs);

	static Vector2Int Min(const Vector2Int& lhs, const Vector2Int& rhs);

	static Vector2Int Perpendicular(const Vector2Int& inDirection);

	static Vector2Int Scale(const Vector2Int& a, const Vector2Int& b);

	void Scale(const Vector2Int& scale);

	static Vector2Int CeilToInt(const Vector2& v);

	static Vector2Int FloorToInt(const Vector2& v);

	int x, y;
};

using vec2i = Vector2Int;

//Non member functions
Vector2Int operator*(const int lhs, const Vector2Int& rhs);
