#pragma once

struct Vector3
{
	//Constructors, Destructor
	Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);
	// Vector3(const Vector4& temp);
	// Vector3(const Vector3& temp) = default;
	// Vector3(Vector3&& temp) = default;

	// Vector3& operator=(const Vector3& rhs) = default;
	// Vector3& operator=(Vector3&& rhs) = default;

	//Arithmetic operators
	Vector3& operator+=(const Vector3& rhs);
	Vector3& operator-=(const Vector3& rhs);
	Vector3& operator*=(const float rhs);
	Vector3& operator/=(const float rhs);

	//Binary operators
	Vector3 operator+(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;
	Vector3 operator*(const float rhs) const;
	Vector3 operator/(const float rhs) const;
	bool operator==(const Vector3& rhs) const;
	bool operator!=(const Vector3& rhs) const;

	float operator*(const Vector3& vec) const; //Dot product
	Vector3 operator^(const Vector3& vec) const; //Cross product

	// Conversion Operators
	// operator glm::vec2();
	// operator physx::PxVec2();
	// operator Vector3();

	//Unary operators
	Vector3 operator-() const;

	//Length
	float Length() const;
	bool IsZero() const;

	//Normalize
	Vector3 Norm() const;
	void Normalize();

	// Convenience Functions
	static Vector3 Zero();
	static Vector3 Up();
	static Vector3 Down();
	static Vector3 Left();
	static Vector3 Right();
	static Vector3 Forward();
	static Vector3 Back();
	static Vector3 One();
	// (+)ve infinity, (-) infinity

	static void Swap(Vector3& lhs, Vector3& rhs);

	static float Distance(const Vector3& ptHead, const Vector3& ptTail);

	static float Angle(const Vector3& from, const Vector3& to); // in degrees

	static Vector3 ClampMagnitude(const Vector3& vector, float maxLength);

	static Vector3 Cross(const Vector3& lhs, const Vector3& rhs);

	static float Dot(const Vector3& lhs, const Vector3& rhs);

	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

	static Vector3 LerpUnclamped(const Vector3& a, const Vector3& b, float t);

	static Vector3 Max(const Vector3& lhs, const Vector3& rhs);

	static Vector3 Min(const Vector3& lhs, const Vector3& rhs);

	// static Vector3 MoveTowards(Vector3& current, const Vector3& target, float maxDistanceDelta);

	// static void OrthoNormalize(Vector3& normal, Vector3& tangent);

	static Vector3 Project(const Vector3& vector, const Vector3& onNormal);

	// static Vector3 ProjectOnPlane(Vector3 vector, Vector3 planeNormal);

	static Vector3 Reflect(const Vector3& inDirection, const Vector3& inNormal);

	// static Vector3 RotateTowards(Vector3 current, Vector3 target, float maxRadiansDelta, float maxMagnitudeDelta);

	static Vector3 Scale(const Vector3& a, const Vector3& b);

	void Scale(const Vector3& scale);

	// static float SignedAngle(const Vector3& from, const Vector3& to);

	// Vector3.SmoothDamp

	static Vector3 Slerp(const Vector3& a, const Vector3& b, float t);

	static Vector3 SlerpUnclamped(const Vector3& a, const Vector3& b, float t);

	float x, y, z;
};

using vec3 = Vector3;

//Non member functions
Vector3 operator*(const float lhs, const Vector3& rhs);
