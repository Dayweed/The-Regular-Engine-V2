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
// warning LNK4099: PDB '' was not found with 'TheRegularEngine.lib(PxPvd.obj)' or at ''; linking object as if no debug info
#pragma warning (disable: 4099)
// warning LNK4006: "__declspec(dllimport) public: __cdecl physx::PxFoundation::PxFoundation(class physx::PxFoundation const &)"
// (__imp_??0PxFoundation@physx@@QEAA@AEBV01@@Z) already defined in PhysX_64.lib(PhysX_64.dll); second definition ignored
#pragma warning (disable: 4006)
#include "PhysX/PxPhysicsAPI.h"

#pragma region Serialization/Deserialization Helper Macros

// only for use in to_json() serialization functions
// does NOT work with non-standard variables like glm::vec3!!
#define WriteMemberToJSON(member) {#member, t.member}

// only for use in to_json() serialization functions
// variant specifically for glm::vec3 members
#define WriteVec3MemberToJSON(member) {#member, std::array<float, 3>{t.member.x, t.member.y, t.member.z}}

// only for use in from_json() deserialization functions
// does NOT work with non-standard variables like glm::vec3!!
#define ReadMemberFromJSON(member)  if (j.contains(#member)) t.member = j.at(#member).get<decltype(t.member)>()

// only for use in from_json() deserialization functions
// variant specifically for glm::vec3 members
#define ReadVec3MemberFromJSON(member) \
	if (j.contains(#member)) t.member = glm::make_vec3(j.at(#member).get<std::array<float, 3>>().data())
#pragma endregion

namespace TRE
{
	struct PhysicsComponentTypes
	{
		enum Enum : short
		{
			Rigidbody       = 1 << 0,
			SphereCollider  = 1 << 1,
			BoxCollider     = 1 << 2,
			CapsuleCollider = 1 << 3,
		};
	};

	struct ForceMode
	{
		enum Enum : short
		{
			Force,				//!< parameter has unit of mass * length / time^2, i.e., a force
			Impulse,			//!< parameter has unit of mass * length / time, i.e., force * time
			VelocityChange,		//!< parameter has unit of length / time, i.e., the effect is mass independent: a velocity change.
			Acceleration		//!< parameter has unit of length/ time^2, i.e., an acceleration. It gets treated just like a force except the mass is not divided out before integration.
		};
	};

	// data that NEEDS to be shared among all physics components of an entity at all times
	struct SharedData
	{
		// physx::PxRigidDynamic* m_RigidDynamic = nullptr;    // pointer to the underlying PhysX actor

		// THIS IS SO TEMPORARY
		physx::PxRigidActor* m_RigidDynamic = nullptr;    // pointer to the underlying PhysX actor
		unsigned m_AttachedComponents : 6 = 0;              // increase bitfield size if necessary!!
		std::string m_GUID{};                               // to know which entity has this component
		bool m_MarkForRemoval = false;                      // bool to activate when this element needs to be removed from m_Actors
	};

	// stuff that EVERY physics component should have
	struct PhysicsComponent
	{
		bool m_IsInitialized = false;
		bool m_IsActive = true;       // whether this component actually participates in the simulation
		bool m_IsDirty = false;
		bool m_IsDestructed = false;
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
		bool m_FreezePositionX = false; bool m_FreezePositionY = false; bool m_FreezePositionZ = false;
		bool m_FreezeRotationX = false; bool m_FreezeRotationY = false; bool m_FreezeRotationZ = false;

		// Serialize
		friend void to_json(nlohmann::json& j, const Rigidbody& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, Rigidbody& t);

		// Allows the base class to get these properties  
		property_vtable()
	};

	// stuff that every collider should have
	struct BaseCollider : PhysicsComponent
	{
		bool m_IsTrigger = false;
		bool m_IsVisible = false; //For rendering of debug collider lines
		glm::vec3 m_Offset = {};
		// physx::PxMaterial* m_PhysicsMaterial = nullptr;
	};

	struct SphereCollider : BaseCollider, property::base
	{
		float m_Radius = 1.0f;

		// Serialize
		friend void to_json(nlohmann::json& j, const SphereCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, SphereCollider& t);

		// Allows the base class to get these properties
		property_vtable()
	};

	struct BoxCollider : BaseCollider, property::base
	{
		glm::vec3 m_HalfExtents = glm::vec3(0.5f);

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

		// Serialize
		friend void to_json(nlohmann::json& j, const CapsuleCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, CapsuleCollider& t);

		// Allows the base class to get these properties
		property_vtable()
	};

	// inline std::tuple<Rigidbody, SphereCollider, BoxCollider> tutu;
	// ^ definition of a global variable in a header file should have the 'inline' specifier
}

property_begin(TRE::Rigidbody)
{
	// property_var(m_IsActive),
	property_var(m_Mass),
	property_var(m_Drag),
	property_var(m_AngularDrag),
	property_var(m_UseGravity),
	property_var(m_IsKinematic),
	property_var(m_FreezePositionX),
	property_var(m_FreezePositionY),
	property_var(m_FreezePositionZ),
	property_var(m_FreezeRotationX),
	property_var(m_FreezeRotationY),
	property_var(m_FreezeRotationZ)
} property_vend_h(TRE::Rigidbody)

property_begin(TRE::SphereCollider)
{
	// property_var(m_IsActive),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius),
	property_var(m_IsVisible)
} property_vend_h(TRE::SphereCollider)

property_begin(TRE::BoxCollider)
{
	// property_var(m_IsActive),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_HalfExtents),
	property_var(m_IsVisible)
} property_vend_h(TRE::BoxCollider)

property_begin(TRE::CapsuleCollider)
{
	// property_var(m_IsActive),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius),
	property_var(m_HalfHeight),
	property_var(m_IsVisible)
} property_vend_h(TRE::CapsuleCollider)
