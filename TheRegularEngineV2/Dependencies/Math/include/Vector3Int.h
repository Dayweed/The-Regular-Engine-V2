#pragma once
#include "Vector3.h"

struct Vector3Int
{
	// Constructors & Destructor
	Vector3Int() = default;
	Vector3Int(const int scalar);
	Vector3Int(const int _x, const int _y, const int _z);
	Vector3Int(const Vector3Int& temp) = default;
	Vector3Int(Vector3Int&& temp) = default;
	~Vector3Int() = default;

	Vector3Int& operator=(const Vector3Int& rhs) = default;
	Vector3Int& operator=(Vector3Int&& rhs) = default;

	//Arithmetic operators
	Vector3Int& operator+=(const Vector3Int& rhs);
	Vector3Int& operator-=(const Vector3Int& rhs);
	Vector3Int& operator*=(const int rhs);
	Vector3Int& operator/=(const int rhs);

	//Binary operators
	Vector3Int operator+(const Vector3Int& rhs) const;
	Vector3Int operator-(const Vector3Int& rhs) const;
	Vector3Int operator*(const int rhs) const;
	Vector3Int operator/(const int rhs) const;
	bool operator==(const Vector3Int& rhs) const;
	bool operator!=(const Vector3Int& rhs) const;

	int operator*(const Vector3Int& vec) const; //Dot product
	Vector3Int operator^(const Vector3Int& vec) const; //Cross product

	// Conversion Operators
	operator glm::vec3() const;
	// operator Vector3Int();

	//Unary operators
	Vector3Int operator-() const;

	//Length
	float Length() const;
	bool IsZero() const;

	// Convenience Functions
	static Vector3Int Zero();
	static Vector3Int Up();
	static Vector3Int Down();
	static Vector3Int Left();
	static Vector3Int Right();
	static Vector3 Forward();
	static Vector3 Back();
	static Vector3Int One();
	// (+)ve infinity, (-) infinity

	static void Swap(Vector3Int& lhs, Vector3Int& rhs);

	static float Distance(const Vector3Int& ptHead, const Vector3Int& ptTail);

	static Vector3Int Max(const Vector3Int& lhs, const Vector3Int& rhs);

	static Vector3Int Min(const Vector3Int& lhs, const Vector3Int& rhs);

	static Vector3Int Scale(const Vector3Int& a, const Vector3Int& b);

	void Scale(const Vector3Int& scale);

	static Vector3Int CeilToInt(const Vector3& v);

	static Vector3Int FloorToInt(const Vector3& v);

	int x{}, y{}, z{};
};

using Vec3i = Vector3Int;

//Non member functions
Vector3Int operator*(const int lhs, const Vector3Int& rhs);
Vector3 operator/(const int lhs, const Vector3Int& rhs);
