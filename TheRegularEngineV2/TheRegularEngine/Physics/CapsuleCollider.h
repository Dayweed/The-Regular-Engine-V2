#pragma once
#include "PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
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
}

property_begin(TRE::CapsuleCollider)
{
	property_var(m_IsVisible),
	property_var(m_IsActive),
	// property_var(m_CollisionLayer),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius),
	property_var(m_HalfHeight),
} property_vend_h(TRE::CapsuleCollider)
