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
#include "Properties.h"
#include "PhysX/PxPhysicsAPI.h"

namespace TRE
{
	struct PhysicsComponentTypes
	{
		enum Enum : short
		{
			Rigidbody		= 1 << 0,
			SphereCollider	= 1 << 1,
			BoxCollider		= 1 << 2,
			CapsuleCollider	= 1 << 3,
		};
	};

	// data that NEEDS to be shared among all physics components of an entity at all times
	struct SharedData
	{
		physx::PxRigidDynamic* m_RigidDynamic = nullptr;
		unsigned m_AttachedComponents : 6 = 0;	// increase bitfield size if necessary!!
		std::string m_GUID{};					// to know which entity has this component
	};

	// stuff that EVERY physics component should have
	struct PhysicsComponent
	{
		bool m_IsInitialized = false, m_IsDestructed = false;
	};

	struct Rigidbody : PhysicsComponent, property::base
	{
		float m_Mass = 1.0f;
		float m_Drag = 0.0f;
		float m_AngularDrag = 0.05f;
		bool m_UseGravity = true;
		bool m_IsKinematic = false;
		// interpolation modes
		// collision detection modes
		// constraints - freeze position x,y,z & rotation x, y, z

		// to write to / read from .json files
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Rigidbody, m_Mass, m_Drag, m_AngularDrag, m_UseGravity, m_IsKinematic);
		// for inside the class, but it's NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE for outside the class
		// but it doesn't work for some reason... :(

		// Allows the base class to get these properties  
		property_vtable()
	};

	// stuff that every collider should have
	struct BaseCollider : PhysicsComponent
	{
		bool m_IsTrigger = false;
		glm::vec3 m_Offset = {};
		// physx::PxMaterial* m_PhysicsMaterial = nullptr;
	};

	struct SphereCollider : BaseCollider, property::base
	{
		float m_Radius = 1.0f;

		// To write to / read from .json files.
		// Can't use NLOHMANN_DEFINE_TYPE_INTRUSIVE because glm::vec3 isn't a type it recognises.
		// Variables that are struct/class can't be handled automatically.
		// So we gotta do it ourselves!

		// Serialize
		friend void to_json(nlohmann::json& j, const SphereCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, SphereCollider& t);

		// Allows the base class to get these properties
		property_vtable()
	};

	struct BoxCollider : BaseCollider, property::base
	{
		Vector3 m_HalfExtents = Vector3(0.5f);

		// To write to / read from .json files.
		// Can't use NLOHMANN_DEFINE_TYPE_INTRUSIVE because glm::vec3 isn't a type it recognises.
		// Variables that are struct/class can't be handled automatically.
		// So we gotta do it ourselves!

		// Serialize
		friend void to_json(nlohmann::json& j, const BoxCollider& t);
		
		// Deserialize
		friend void from_json(const nlohmann::json& j, BoxCollider& t);

		// Allows the base class to get these properties  
		property_vtable()
	};

	struct CapsuleCollider : BaseCollider, property::base
	{
		float m_Radius = 1.0f;
		float m_HalfHeight = 0.5f;

		// To write to / read from .json files.
		// Can't use NLOHMANN_DEFINE_TYPE_INTRUSIVE because glm::vec3 isn't a type it recognises.
		// Variables that are struct/class can't be handled automatically.
		// So we gotta do it ourselves!

		// Serialize
		friend void to_json(nlohmann::json& j, const SphereCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, SphereCollider& t);

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

property_begin(TRE::CapsuleCollider)
{
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius),
	property_var(m_HalfHeight)
} property_vend_h(TRE::CapsuleCollider)

// none			- just don't have anything, please
// collider		- make shape with 
// rigidbody	- 
// both			- create stuff like prior

// WHAT IF A THING DIDN'T HAVE TO HAVE A SHAPE ATTACHED???
// or what if I shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); ?
// or maybe rb.m_RigidDynamic->setActorFlags(PxActorFlag::eDISABLE_GRAVITY);

// WAIT I NEED THE RIGIDACTOR AN ATTACHED COMPONENTS TO BE SHARED **AT ALL TIMES**
