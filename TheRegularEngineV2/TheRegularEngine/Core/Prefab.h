#pragma once

#include "pch.h"
#include "ECS.h"

namespace TRE
{
	struct Prefabing
	{
		std::string m_Base{};																// ID of file name (.json)
		std::vector<std::string> m_Instances{};												// Instances that are based on this Entity (ID of file name (.json))
		// std::pair<Component Name, std::vector<Data Var Name>>
		std::vector<std::pair<std::string, std::vector<std::string>>> m_Overrides{};		// List of components/properties added/removed/overriten for a given component
	};

	class PrefabManager : public ECSSystem
	{

	};
}