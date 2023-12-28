#pragma once
#include "PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
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
}

property_begin(TRE::SphereCollider)
{
	property_var(m_IsVisible)      .Name("Is Visible"),
	property_var(m_IsActive)       .Name("Is Active"),
	property_var(m_CollisionLayer) .Name("Layer"),
	property_var(m_IsTrigger)      .Name("Is Trigger"),
	property_var(m_Offset)         .Name("Offset"),
	property_var(m_Radius)         .Name("Radius"),
} property_vend_h(TRE::SphereCollider)
