#pragma once

#include "Core/ECS.h"
#include "Core/System.h"

namespace TRE
{
	class ScriptComponent: property::base
	{
	public:
		
		std::string m_StoredClass;
		std::string m_GUID;
		bool m_IsDirty{ false };
		bool m_RanStart{ false };
		property_vtable() 

		ScriptComponent()= default;
		ScriptComponent(const std::string&);


		friend void to_json(nlohmann::json& j, const ScriptComponent& s) // Serialize
		{
			j = nlohmann::json
			{
				{ "m_StoredClass", s.m_StoredClass }
			};
		}

		friend void from_json(const nlohmann::json& j , ScriptComponent& s) // Deserialize
		{
			
			s.m_StoredClass = j.at("m_StoredClass").get<std::string>();
			s.m_IsDirty = true;
		}

	private:

		std::string m_NameSpace;
		std::string m_ClassName;

		std::string ExtractNameSpace(const std::string&);
		std::string ExtractClassName(const std::string&);

	};
}

property_begin(TRE::ScriptComponent)
{
	property_var(m_StoredClass)

} property_vend_h(TRE::ScriptComponent)
