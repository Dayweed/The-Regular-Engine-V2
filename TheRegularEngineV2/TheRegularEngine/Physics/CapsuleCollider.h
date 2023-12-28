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
	property_var(m_IsVisible)      .Name("Is Visible"),
	property_var(m_IsActive)       .Name("Is Active"),
	property_var(m_CollisionLayer) .Name("Layer"),
	property_var(m_IsTrigger)      .Name("Is Trigger"),
	property_var(m_Offset)         .Name("Offset"),
	property_var(m_Radius)         .Name("Radius"),
	property_var(m_HalfHeight)     .Name("Half Height"),
} property_vend_h(TRE::CapsuleCollider)
