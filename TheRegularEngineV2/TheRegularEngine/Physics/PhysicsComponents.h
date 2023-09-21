/*!
	@file      PhysicsComponents.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      14/09/2023
	@brief     This file contains the definition of several physics
			   components, such as Rigidbody and various colliders.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "PhysX/PxPhysicsAPI.h"
#include "Vector3.h"

struct PhysicsComponentTypes
{
	enum Enum : short
	{
		Rigidbody		= 1 << 0,
		SphereCollider	= 1 << 1,
		BoxCollider		= 1 << 2,
	};
};

// data that NEEDS to be shared among all physics components of an entity at all times
struct SharedData
{
	physx::PxRigidDynamic* m_RigidDynamic = nullptr;
	unsigned m_AttachedComponents : 6 = 0;
	// increase bitfield size if necessary!!
};

struct Rigidbody
{
	float m_Mass = 1.0f;
	float m_Drag = 0.0f;
	float m_AngularDrag = 0.05f;
	bool m_UseGravity = true;
	bool m_IsKinematic = false;
	// interpolation modes
	// collision detection modes
	// constraints - freeze position x,y,z & rotation x, y, z
};

struct BaseCollider
{
	bool m_IsTrigger = false;
	// physx::PxMaterial* m_PhysicsMaterial = nullptr;
};

struct SphereCollider : BaseCollider
{
	Vector3 m_Offset = {};
	float m_Radius = 1.0f;
};

struct BoxCollider : BaseCollider
{
	Vector3 m_Offset = {};
	Vector3 m_HalfExtents = Vector3(0.5f);
};

// none			- just don't have anything, please
// collider		- make shape with 
// rigidbody	- 
// both			- create stuff like prior

// WHAT IF A THING DIDN'T HAVE TO HAVE A SHAPE ATTACHED???
// or what if I shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); ?
// or maybe rb.m_RigidDynamic->setActorFlags(PxActorFlag::eDISABLE_GRAVITY);

// WAIT I NEED THE RIGIDACTOR AN ATTACHED COMPONENTS TO BE SHARED **AT ALL TIMES**
