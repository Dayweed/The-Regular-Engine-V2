#include "pch.h"
#include "TREIncludes.h"


namespace TRE
{
	std::string PersistentManager::GetVariableValue(std::string variableName)
	{
		if (m_Variables.find(variableName) == m_Variables.end())
		{
			m_Variables[variableName] = "";
		}

		return m_Variables[variableName];
	}

	void PersistentManager::SetVariableValue(std::string variableName, std::string variableValue)
	{
		m_Variables[variableName] = variableValue;
	}

	void PersistentManager::ClearVariables()
	{
		m_Variables.clear();
	}
}