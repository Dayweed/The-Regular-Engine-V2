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

namespace TRE
{
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
		std::string m_GUID{}; // to know which entity has this component
	};

	struct Rigidbody : property::base
	{
		bool m_IsInitialized = false;

		float m_Mass = 1.0f;
		float m_Drag = 0.0f;
		float m_AngularDrag = 0.05f;
		bool m_UseGravity = true;
		bool m_IsKinematic = false;
		// interpolation modes
		// collision detection modes
		// constraints - freeze position x,y,z & rotation x, y, z

		// Allows the base class to get these properties  
		property_vtable()
	};

	struct BaseCollider
	{
		bool m_IsInitialized = false;

		bool m_IsTrigger = false;
		// physx::PxMaterial* m_PhysicsMaterial = nullptr;
	};

	struct SphereCollider : BaseCollider, property::base
	{
		Vector3 m_Offset = {};
		float m_Radius = 1.0f;

		// Allows the base class to get these properties
		property_vtable()
	};

	struct BoxCollider : BaseCollider, property::base
	{
		Vector3 m_Offset = {};
		Vector3 m_HalfExtents = Vector3(0.5f);

		// Allows the base class to get these properties  
		property_vtable()
	};

	// inline std::tuple<Rigidbody, SphereCollider, BoxCollider> tutu;
	// ^ definition of a global variable in a header file should have the 'inline' specifier

}

property_begin(TRE::Rigidbody)
{
	property_var(m_Mass),
	property_var(m_Drag),
	property_var(m_AngularDrag),
	property_var(m_UseGravity),
	property_var(m_IsKinematic)

} property_vend_h(TRE::Rigidbody)

property_begin(TRE::SphereCollider)
{
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius)
} property_vend_h(TRE::SphereCollider)

property_begin(TRE::BoxCollider)
{
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_HalfExtents)
} property_vend_h(TRE::BoxCollider)

// none			- just don't have anything, please
// collider		- make shape with 
// rigidbody	- 
// both			- create stuff like prior

// WHAT IF A THING DIDN'T HAVE TO HAVE A SHAPE ATTACHED???
// or what if I shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); ?
// or maybe rb.m_RigidDynamic->setActorFlags(PxActorFlag::eDISABLE_GRAVITY);

// WAIT I NEED THE RIGIDACTOR AN ATTACHED COMPONENTS TO BE SHARED **AT ALL TIMES**
