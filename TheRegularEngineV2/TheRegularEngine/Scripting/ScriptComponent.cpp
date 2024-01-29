#include "pch.h"
#include "ScriptComponent.h"

#include <cmath>

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
		// update the m_storedClass
		m_StoredClass = "";
		for(auto j: m_RegisteredScripts)
		{
			if(j == m_RegisteredScripts.back())
			{
				m_StoredClass += j;
			}
			else
			{
				m_StoredClass += j + ",";
			}
		}

		TRE_INFO("Script {0} added to the script component", _class);

		return true;
	}

	bool ScriptComponent::RemoveScriptFromComponent(std::string _class)
	{
		//check if the script exists, if yes remove it
		for(int i = 0; i < m_RegisteredScripts.size(); i++)
		{
			if(m_RegisteredScripts[i] == _class)
			{
				m_RegisteredScripts.erase(m_RegisteredScripts.begin() + i);

				m_StoredClass = "";
				for(auto j: m_RegisteredScripts)
				{
					if(j == m_RegisteredScripts.back())
					{
						m_StoredClass += j;
					}
					else
					{
						m_StoredClass += j + ",";
					}
				}

				return true;
			}
		}

		// if not return false
		TRE_INFO("Script {0} does not exist in the script component", _class);
		return false;
	}


}
