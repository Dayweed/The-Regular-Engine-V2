#pragma once
#include "Physics/PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
	struct SphereCollider : BaseCollider, property::base
	{
		float m_Radius = 1.0f;

		// Serialize
		friend void to_json(nlohmann::json& j, const SphereCollider& t)
		{
			j = nlohmann::json{
				WriteMemberToJSON(m_IsActive),
				WriteMemberToJSON(m_IsTrigger),
				WriteMemberToJSON(m_CollisionLayer.m_LayerID),
				WriteMemberToJSON(m_PhysicsMaterial.m_MaterialID),
				WriteVec3MemberToJSON(m_Offset),
				WriteMemberToJSON(m_Radius),
			};
		}

		// Deserialize
		friend void from_json(const nlohmann::json& j, SphereCollider& t)
		{
			ReadMemberFromJSON(m_IsActive);
			ReadMemberFromJSON(m_IsTrigger);
			ReadMemberFromJSON(m_CollisionLayer.m_LayerID);
			ReadMemberFromJSON(m_PhysicsMaterial.m_MaterialID);
			ReadVec3MemberFromJSON(m_Offset);
			ReadMemberFromJSON(m_Radius);
		}

		// Allows the base class to get these properties
		property_vtable()
	};
}

property_begin(TRE::SphereCollider)
{
	property_var(m_IsActive),
	property_var(m_IsVisible),
	property_var(m_CollisionLayer),
	property_var(m_PhysicsMaterial),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius)
} property_vend_h(TRE::SphereCollider)
