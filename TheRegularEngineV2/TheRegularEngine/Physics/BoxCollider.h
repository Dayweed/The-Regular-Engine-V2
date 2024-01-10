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

		// Allows the base class to get these properties  
		property_vtable()
	};
}

// I don't know how. I don't know why. But using .Name() after property_var() (like you're meant to) ruined several colliders.
// I SPENT OVER TWO HOURS TRACKING THIS BUG DOWN, LINE BY LINE, CHANGE BY CHANGE. TOMAS WHYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
property_begin(TRE::BoxCollider)
{
	property_var(m_IsActive),
	property_var(m_IsVisible),
	property_var(m_CollisionLayer),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_HalfExtents)
} property_vend_h(TRE::BoxCollider)
