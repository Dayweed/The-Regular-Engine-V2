#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	class ParticleComponent : property::base
	{
		public:
			bool m_IsVisible = false;

			property_vtable()

				friend void to_json(nlohmann::json& j, const ParticleComponent& t)
			{

				j = nlohmann::json
				{
					{ "m_IsVisible", t.m_IsVisible },
				};
			}

			friend void from_json(const nlohmann::json& j, ParticleComponent& t)
			{
				if (j.contains("m_IsVisible"))
				{
					t.m_IsVisible = j.at("m_IsVisible").get<bool>();
				}
			}
	};
}

property_begin(TRE::ParticleComponent)
{
	property_var(m_IsVisible),

} property_vend_h(TRE::ParticleComponent)