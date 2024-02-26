#pragma once
#include "Core/ECS.h"

namespace TRE
{
	struct Parenting : property::base
	{
		std::string m_Parent{};
		std::vector<std::string> m_Children{};
		bool m_IsDirty{ false };

		Parenting() = default;
		~Parenting() = default;

		property_vtable()           // Allows the base class to get these properties  

			//NLOHMANN_DEFINE_TYPE_INTRUSIVE(Parenting, m_Parent, m_Children)

			friend void to_json(nlohmann::json& j, const Parenting& t)
		{
			j = nlohmann::json{
				{ "m_Parent", t.m_Parent},
				{ "m_Children", t.m_Children }
			};
		}
		friend void from_json(const nlohmann::json& j, Parenting& t)
		{
			if (j.contains("m_Parent"))
				t.m_Parent = j.at("m_Parent").get<std::string>();
			if (j.contains("m_Children"))
				t.m_Children = j.at("m_Children").get<std::vector<std::string>>();

			t.m_IsDirty = true;
		}
	};
}

property_begin(TRE::Parenting)
{
	property_var_fnbegin("Parent", std::string)
	{
		if (isRead)
		{
			InOut = Self.m_Parent;
		}

	} property_var_fnend(),
		property_var_fnbegin("Parent Name", std::string)
	{
		if (isRead)
		{
			TRE::Entity parent{ TRE::ECSManager::Instance().FindEntity(Self.m_Parent) };
			InOut = parent ? parent->GetName() : "";
		}

	} property_var_fnend(),
		property_var_fnbegin("Children Size", int)
	{
		if (isRead)
		{
			InOut = static_cast<int>(Self.m_Children.size());
		}

	} property_var_fnend(),
} property_vend_h(TRE::Parenting)