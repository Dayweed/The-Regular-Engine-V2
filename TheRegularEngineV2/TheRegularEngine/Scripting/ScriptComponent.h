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

		std::vector<std::string> m_RegisteredScripts;

		property_vtable() 

		ScriptComponent()= default;
		ScriptComponent(const std::string&);

		bool AddScriptToComponent(std::string);


		friend void to_json(nlohmann::json& j, const ScriptComponent& s) // Serialize
		{
			j = nlohmann::json
			{
				{ "m_StoredClass", s.m_StoredClass },
				{ "m_RegisteredScripts", s.m_RegisteredScripts}
			};
		}

		friend void from_json(const nlohmann::json& j , ScriptComponent& s) // Deserialize
		{
			
			s.m_StoredClass = j.at("m_StoredClass").get<std::string>();
			//s.m_RegisteredScripts = j.at("m_RegisteredScripts").get<std::map<std::string, bool>>();
			s.m_IsDirty = true;
		}


	};
}

property_begin(TRE::ScriptComponent)
{
	property_var(m_StoredClass)

} property_vend_h(TRE::ScriptComponent)
