#pragma once

struct Vector2
{
	//Constructors, Destructor
	Vector2(float _x = 0.f, float _y = 0.f);
	Vector2(const Vector2& temp);
	// Vector2(const Vector3& temp);

	//Arithmetic operators
	Vector2& operator=(const Vector2& rhs);
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

	// Conversion Operators
	// operator glm::vec2();
	// operator physx::PxVec2();
	// operator Vector3();

	//Unary operators
	Vector2 operator-() const;

	//Length
	float Length() const;

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

	static float Angle(const Vector2& from, const Vector2& to);

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

	float x, y;
};

using vec2 = Vector2;

//Non member functions
float operator*(const Vector2& vec0, const Vector2& vec1); //Dot product

float operator^(const Vector2& vec0, const Vector2& vec1); //Cross product

Vector2 operator*(const float lhs, const Vector2& rhs);
