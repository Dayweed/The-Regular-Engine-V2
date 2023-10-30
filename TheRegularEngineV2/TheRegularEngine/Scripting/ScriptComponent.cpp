#include "pch.h"
#include "ScriptComponent.h"
#include "ScriptEngine.h"


namespace TRE
{
	ScriptComponent::ScriptComponent(const std::string& _class)
	{
		// Split the string
		m_StoredClass = _class;

		// Since we want users to type in with the format of {namespace}.{classname} , we will split the string by
		// the dot and store it separately
		m_NameSpace = ExtractNameSpace(_class);
		m_ClassName = ExtractClassName(_class);

	}

	std::string ScriptComponent::ExtractNameSpace(const std::string& _class)
	{
		// Split the string, take the first part
		std::string delimiter = ".";
		std::string token = _class.substr(0, _class.find(delimiter));
		return token;
	}

	std::string ScriptComponent::ExtractClassName(const std::string& _class)
	{
		std::string delimiter = ".";
		std::string temp = _class;
		temp.erase(0, temp.find(delimiter) + delimiter.length());
		return temp;
	}


}
