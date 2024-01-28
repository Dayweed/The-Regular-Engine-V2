#pragma once
#include "PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
	struct CylinderCollider : BaseCollider, property::base
	{
		float m_Radius = 1.0f;
		float m_Height = 1.0f;

		// Serialize
		friend void to_json(nlohmann::json& j, const CylinderCollider& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, CylinderCollider& t);

		// Allows the base class to get these properties
		property_vtable()
	};
}

property_begin(TRE::CylinderCollider)
{
	property_var(m_IsActive),
	property_var(m_IsVisible),
	property_var(m_CollisionLayer),
	property_var(m_PhysicsMaterial),
	property_var(m_IsTrigger),
	property_var(m_Offset),
	property_var(m_Radius),
	property_var(m_Height)
} property_vend_h(TRE::CylinderCollider)
