#pragma once
#include "PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
	struct BoxCollider : BaseCollider, property::base
	{
		glm::vec3 m_HalfExtents = glm::vec3(0.5f);

		// Serialize
		friend void to_json(nlohmann::json& j, const BoxCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, BoxCollider& t);

		CollisionLayer m_CollisionLayer;

		// Allows the base class to get these properties  
		property_vtable()
	};
}

property_begin(TRE::BoxCollider)
{
	property_var(m_IsVisible)      .Name("Is Visible"),
	property_var(m_IsActive)       .Name("Is Active"),
	property_var(m_CollisionLayer) .Name("Layer"),
	property_var(m_IsTrigger)      .Name("Is Trigger"),
	property_var(m_Offset)         .Name("Offset"),
	property_var(m_HalfExtents)    .Name("Half Extents"),
} property_vend_h(TRE::BoxCollider)
