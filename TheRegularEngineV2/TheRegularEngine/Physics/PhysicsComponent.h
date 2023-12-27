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

// only for use in to_json() serialization functions, does NOT work with non-standard variables like glm::vec3!!
#define WriteMemberToJSON(member) {#member, t.member}

// only for use in to_json() serialization functions, variant specifically for glm::vec3 members
#define WriteVec3MemberToJSON(member) {#member, std::array<float, 3>{t.member.x, t.member.y, t.member.z}}

// only for use in from_json() deserialization functions, does NOT work with non-standard variables like glm::vec3!!
#define ReadMemberFromJSON(member) if (j.contains(#member)) t.member = j.at(#member).get<decltype(t.member)>()

// only for use in from_json() deserialization functions, variant specifically for glm::vec3 members
#define ReadVec3MemberFromJSON(member) if (j.contains(#member)) t.member = glm::make_vec3(j.at(#member).get<std::array<float, 3>>().data())
#pragma endregion

// basically an enum with a static array of its value's names, since enum reflection isn't really a thing
struct CollisionLayer
{
	enum Layer
	{
		Default, Layer01, Layer02, Layer03,
		Layer04, Layer05, Layer06, Layer07,
		Layer08, Layer09, Layer10, Layer11,
		Layer12, Layer13, Layer14, Layer15,
		//Layer16, Layer17, Layer18, Layer19,
		//Layer20, Layer21, Layer22, Layer23,
		//Layer24, Layer25, Layer26, Layer27,
		//Layer28, Layer29, Layer30, Layer31,
	};

	// first = name, second = enum value (i'm not sure i like this...)
	constexpr static std::array m_LayerNameList
	{
		std::pair {"Default", Default}, std::pair {"Layer01", Layer01}, std::pair {"Layer02", Layer02}, std::pair {"Layer03", Layer03},
		std::pair {"Layer04", Layer04}, std::pair {"Layer05", Layer05}, std::pair {"Layer06", Layer06}, std::pair {"Layer07", Layer07},
		std::pair {"Layer08", Layer08}, std::pair {"Layer09", Layer09}, std::pair {"Layer10", Layer10}, std::pair {"Layer11", Layer11},
		std::pair {"Layer12", Layer12}, std::pair {"Layer13", Layer13}, std::pair {"Layer14", Layer14}, std::pair {"Layer15", Layer15},
		//std::pair {"Layer16", Layer16}, std::pair {"Layer17", Layer17}, std::pair {"Layer18", Layer18}, std::pair {"Layer19", Layer19},
		//std::pair {"Layer20", Layer20}, std::pair {"Layer21", Layer21}, std::pair {"Layer22", Layer22}, std::pair {"Layer23", Layer23},
		//std::pair {"Layer24", Layer24}, std::pair {"Layer25", Layer25}, std::pair {"Layer26", Layer26}, std::pair {"Layer27", Layer27},
		//std::pair {"Layer28", Layer28}, std::pair {"Layer29", Layer29}, std::pair {"Layer30", Layer30}, std::pair {"Layer31", Layer31},
	};

	Layer m_LayerID = Default;
};

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

		// CollisionLayer m_CollisionLayer; // collision layer of object
	};
}
