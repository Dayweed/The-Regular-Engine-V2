#pragma once

#include "Core/ECS.h"
#include "Core/System.h"

namespace TRE
{
	class ScriptComponent: property::base
	{
	public:
		std::string m_ScriptName;
		std::string m_ScriptPath;
		std::string m_GUID;
		bool m_IsDirty{ true };

		std::string GetScriptParentGUID();

		friend void to_json(nlohmann::json& j, const ScriptComponent& s) // Serialize
		{
			j = nlohmann::json
			{
				{ "m_ScriptName", s.m_ScriptName },
				{ "m_ScriptPath", s.m_ScriptPath },
				{ "m_GUID", s.m_GUID }
			};
		}

		friend void from_json(const nlohmann::json& j , ScriptComponent& s) // Deserialize
		{
			s.m_ScriptName = j.at("m_ScriptName").get<std::string>();
			s.m_ScriptPath = j.at("m_ScriptPath").get<std::string>();
			s.m_GUID = j.at("m_GUID").get<std::string>();
		}

	};
}

property_begin(TRE::ScriptComponent)
{
	property_var(m_ScriptName)
	, property_var(m_ScriptPath)
	, property_var(m_GUID)

} property_vend_h(TRE::ScriptComponent)
}