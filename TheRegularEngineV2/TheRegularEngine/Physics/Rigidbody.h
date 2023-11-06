#pragma once
#include "PhysicsComponent.h"
#include "Properties.h"

namespace TRE
{
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
		bool m_FreezePositionX = false; bool m_FreezePositionY = false; bool m_FreezePositionZ = false;
		bool m_FreezeRotationX = false; bool m_FreezeRotationY = false; bool m_FreezeRotationZ = false;

		// Serialize
		friend void to_json(nlohmann::json& j, const Rigidbody& t);

		// Deserialize
		friend void from_json(const nlohmann::json& j, Rigidbody& t);

		// Allows the base class to get these properties  
		property_vtable()
	};

	struct ForceMode
	{
		enum Enum : short
		{
			Force,          //!< parameter has unit of mass * length / time^2, i.e., a force
			Impulse,        //!< parameter has unit of mass * length / time, i.e., force * time
			VelocityChange, //!< parameter has unit of length / time, i.e., the effect is mass independent: a velocity change.
			Acceleration    //!< parameter has unit of length/ time^2, i.e., an acceleration. It gets treated just like a force except the mass is not divided out before integration.
		};
	};
}

property_begin(TRE::Rigidbody)
{
	property_var(m_IsActive),
	property_var(m_Mass),
	property_var(m_Drag),
	property_var(m_AngularDrag),
	property_var(m_UseGravity),
	property_var(m_IsKinematic),
	property_var(m_FreezePositionX),
	property_var(m_FreezePositionY),
	property_var(m_FreezePositionZ),
	property_var(m_FreezeRotationX),
	property_var(m_FreezeRotationY),
	property_var(m_FreezeRotationZ)
} property_vend_h(TRE::Rigidbody)
