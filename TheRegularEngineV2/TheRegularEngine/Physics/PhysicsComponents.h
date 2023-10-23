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
		bool m_IsDirty = false;

		// To write to / read from .json files.
		// Can't use NLOHMANN_DEFINE_TYPE_INTRUSIVE because glm::vec3 isn't a type it recognises.
		// Variables that are struct/class can't be handled automatically.
		// So we gotta do it ourselves!

		friend void to_json(nlohmann::json& j, const SphereCollider& t) // Serialize
		{
			const std::vector<float> v_offset{ t.m_Offset.x, t.m_Offset.y, t.m_Offset.z };

			j = nlohmann::json{
				{ "m_Offset", v_offset },
				{ "m_Radius", t.m_Radius },
				{ "m_IsTrigger", t.m_IsTrigger }
			};
		}

		friend void from_json(const nlohmann::json& j, SphereCollider& t) // Deserialize
		{
			const std::vector<float> v_off{ j.at("m_Offset").get<std::vector<float>>() };
			const float a_off[3]{ v_off[0], v_off[1], v_off[2] };
			t.m_Offset = glm::make_vec3(a_off);

			t.m_Radius = j.at("m_Radius").get<float>();
			t.m_IsTrigger = j.at("m_IsTrigger").get<bool>();
		}

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

		friend void to_json(nlohmann::json& j, const BoxCollider& t) // Serialize
		{
			const std::vector<float> v_offset{ t.m_Offset.x, t.m_Offset.y, t.m_Offset.z };
			const std::vector<float> v_halfEx{ t.m_HalfExtents.x, t.m_HalfExtents.y, t.m_HalfExtents.z };

			j = nlohmann::json{
				{ "m_Offset", v_offset },
				{ "m_HalfExtents", v_halfEx },
				{ "m_IsTrigger", t.m_IsTrigger }
			};
		}

		friend void from_json(const nlohmann::json& j, BoxCollider& t) // Deserialize
		{
			const std::vector<float> v_off{ j.at("m_Offset").get<std::vector<float>>() };
			const float a_off[3]{ v_off[0], v_off[1], v_off[2] };
			t.m_Offset = glm::make_vec3(a_off);

			const std::vector<float> v_half{ j.at("m_HalfExtents").get<std::vector<float>>() };
			const float a_half[3]{ v_half[0], v_half[1], v_half[2] };
			t.m_HalfExtents = glm::make_vec3(a_half);

			t.m_IsTrigger = j.at("m_IsTrigger").get<bool>();
		}

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
