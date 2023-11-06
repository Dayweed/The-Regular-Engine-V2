/*!
	@file      PhysicsComponent.h
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
// warning LNK4099: PDB '' was not found with 'TheRegularEngine.lib(PxPvd.obj)' or at ''; linking object as if no debug info
#pragma warning (disable: 4099)
// warning LNK4006: "__declspec(dllimport) public: __cdecl physx::PxFoundation::PxFoundation(class physx::PxFoundation const &)"
// (__imp_??0PxFoundation@physx@@QEAA@AEBV01@@Z) already defined in PhysX_64.lib(PhysX_64.dll); second definition ignored
#pragma warning (disable: 4006)
#include "PhysX/PxPhysicsAPI.h"

#pragma region Serialization/Deserialization Helper Macros
#include "nlohmann/json.hpp"
#include "glm/glm.hpp"

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

	// data that NEEDS to be shared among all physics components of an entity at all times
	struct SharedData
	{
		physx::PxRigidDynamic* m_RigidDynamic = nullptr;    // pointer to the underlying PhysX actor
		unsigned m_AttachedComponents : 6 = 0;              // increase bitfield size if necessary!!
		std::string m_GUID{};                               // to know which entity has this component
		bool m_MarkForRemoval = false;                      // bool to activate when this element needs to be removed from m_Actors
	};

	// stuff that EVERY physics component should have
	struct PhysicsComponent
	{
		bool m_IsInitialized = false; // used to ensure Construct##Comp() only runs once
		bool m_IsActive = true;       // whether this component actually participates in the simulation
		bool m_IsDirty = false;       // whether the values of this component have been modified
		bool m_IsDestructed = false;  // used to check if Destruct##Comp() has been executed for this component
	};

	// stuff that every COLLIDER should have (not Rigidbody)
	struct BaseCollider : PhysicsComponent
	{
		bool m_IsTrigger = false;
		bool m_IsVisible = false; // For rendering of debug collider lines
		glm::vec3 m_Offset = {};
		// physx::PxMaterial* m_PhysicsMaterial = nullptr;
	};
}
