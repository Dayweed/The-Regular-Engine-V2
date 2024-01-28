#include "pch.h"
#include "ScriptComponent.h"
#include "ScriptEngine.h"


namespace TRE
{
	ScriptComponent::ScriptComponent(const std::string& _class)
	{
		// Split the string
		m_StoredClass = _class;

	}

	bool ScriptComponent::AddScriptToComponent(std::string _class)
	{
		//check if the script is already added
		for(auto i : m_RegisteredScripts)
		{
			if(i == _class)
			{
				TRE_INFO("Script {0} already exists in the script component", _class);
				return false;
			}
		}

		// add in the script
		m_RegisteredScripts.push_back(_class);
		return true;
	}


}
