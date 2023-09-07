#include "pch.h"
#include "Serialization.h"

namespace TRE
{
	void ReadExternalFile(rapidjson::Document& document, const char* filename)
	{
		std::string folderpath = "../";
		std::string filepath = folderpath + filename;
		std::ifstream file(filepath);
		//std::cout << "[ReadExternalFile] File " << filename << " not found\n";
		//TRE_ASSERT(file.is_open(), "[ReadExternalFile] File " + filename + " not found");

		rapidjson::IStreamWrapper isw(file);
		document.ParseStream(isw);
	}

	void WriteToExternalFile(rapidjson::Document& document, const char* filename)
	{
		std::string folderpath = "../";
		std::string filepath = folderpath + filename;
		std::ofstream file(filepath);

		rapidjson::OStreamWrapper osw(file);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		document.Accept(writer);
	}

	// Object Serializer
	ObjectSerializer::ObjectSerializer()
	{
		m_document.SetObject();
	}

	ObjectSerializer::ObjectSerializer(const char* filename)
	{
		m_document.SetObject();
		ReadExternalFile(m_document, filename);
	}

	ObjectSerializer::~ObjectSerializer()
	{
		m_map.~vector();
		m_array.~vector();
		m_object.~vector();
		m_document.SetNull();
	}

	bool ObjectSerializer::updateObjectInDocument(const char* object, const char* component, const char* variable, int value)
	{
		if (m_document.HasMember(object))
		{
			if (m_document[object].HasMember(component))
			{
				if (m_document[object][component].HasMember(variable))
				{
					m_document[object][component][variable] = value;
					return true;
				}
			}
		}
		return false;
	}

	bool ObjectSerializer::updateObjectInDocument(const char* object, const char* component, const char* variable, float value)
	{
		if (m_document.HasMember(object))
		{
			if (m_document[object].HasMember(component))
			{
				if (m_document[object][component].HasMember(variable))
				{
					m_document[object][component][variable] = value;
					return true;
				}
			}
		}
		return false;
	}

	bool ObjectSerializer::updateObjectInDocument(const char* object, const char* component, const char* variable, double value)
	{
		if (m_document.HasMember(object))
		{
			if (m_document[object].HasMember(component))
			{
				if (m_document[object][component].HasMember(variable))
				{
					m_document[object][component][variable] = value;
					return true;
				}
			}
		}
		return false;
	}

	bool ObjectSerializer::updateObjectInDocument(const char* object, const char* component, const char* variable, std::string value)
	{
		if (m_document.HasMember(object))
		{
			if (m_document[object].HasMember(component))
			{
				if (m_document[object][component].HasMember(variable))
				{
					m_document[object][component][variable].SetString(value.c_str(), m_document.GetAllocator());
					return true;
				}
			}
		}
		return false;
	}

	bool ObjectSerializer::updateObjectInDocument(const char* object, const char* component, const char* variable, bool value)
	{
		if (m_document.HasMember(object))
		{
			if (m_document[object].HasMember(component))
			{
				if (m_document[object][component].HasMember(variable))
				{
					m_document[object][component][variable] = value;
					return true;
				}
			}
		}
		return false;
	}

	void ObjectSerializer::writeToDoc(const char* filename)
	{
		WriteToExternalFile(m_document, filename);
	}

	void ObjectSerializer::AddObjectToDoc(rapidjson::Value& v_value, const char* name)
	{
		//get allocator
		rapidjson::Document::AllocatorType& allocator = m_document.GetAllocator();

		//add the object to the document if it exists, update the values that are in the object
		if (m_document.HasMember(name))
		{
			//iterate through the m_value and check if the key exists in the document
			for (auto i = v_value.MemberBegin(); i != v_value.MemberEnd(); ++i)
			{
				//if it does, update the value
				if (m_document[name].HasMember(i->name.GetString()))
				{
					//iterate through the m_value's value and check if the key exists in the document
					for (auto j = i->value.MemberBegin(); j != i->value.MemberEnd(); ++j)
					{
						//if it does, update the value
						if (m_document[name][i->name.GetString()].HasMember(j->name.GetString()))
						{
							m_document[name][i->name.GetString()][j->name.GetString()].CopyFrom(j->value, allocator);
						}
						else
						{
							//if it doesn't, add the value
							m_document[name][i->name.GetString()].AddMember(j->name, j->value, allocator);
						}
					}
				}
				else
				{
					//if it doesn't, add the value
					m_document[name].AddMember(i->name, i->value, allocator);
				}
			}

		}
		else
		{
			//add the object to the document
			rapidjson::Value key;
			key.SetString(name, allocator);
			m_document.AddMember(key, v_value, allocator);
		}
	}

	rapidjson::Document& ObjectSerializer::getDoc()
	{
		return m_document;
	}


	// Object Deserializer

	ObjectDeserializer::ObjectDeserializer()
	{
		m_document.SetObject();
	}

	ObjectDeserializer::ObjectDeserializer(const char* filename)
	{
		ReadExternalFile(m_document, filename);
		indexDocument();
	}

	ObjectDeserializer::~ObjectDeserializer()
	{
		m_document.SetNull();
		m_objectnames.clear();
		componentMap.clear();
		variableMap.clear();
	}

	void ObjectDeserializer::loadFile(const char* filename)
	{
		//clear the document
		m_document.SetNull();
		std::string folderpath = "../";
		std::string filepath = folderpath + filename;
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			std::cout << "[loadFile] Error: File " << filepath << " not found" << std::endl; // Change to assertion
			//LogFatal("[loadFile] Error: File " + filepath + " not found");
			return;
		}

		rapidjson::IStreamWrapper isw(file);
		m_document.ParseStream(isw);
	}

	void ObjectDeserializer::loadString(const char* string)
	{
		//parse string
		m_document.Parse(string);
	}

	void ObjectDeserializer::loadDocument(rapidjson::Document doc)
	{
		//copy document
		m_document.CopyFrom(doc, m_document.GetAllocator());
	}

	void ObjectDeserializer::loadValue(rapidjson::Value val)
	{
		//copy value
		m_document.CopyFrom(val, m_document.GetAllocator());
	}

	void ObjectDeserializer::loadValue(rapidjson::Value* val)
	{
		//copy value
		m_document.CopyFrom(*val, m_document.GetAllocator());
	}

	bool ObjectDeserializer::checkComponentIfObject(std::string object, std::string name)
	{
		if (m_document[object.c_str()][name.c_str()].IsObject())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void ObjectDeserializer::indexDocument()
	{
		//clear the 3 vectors that will contain the objects, components and variables
		m_objectnames.clear();
		componentMap.clear();
		variableMap.clear();
		std::string componentTemp, variableTemp;

		//using a for loop, go through the document and index all the objects , components and variables

		for (auto i = m_document.MemberBegin(); i != m_document.MemberEnd(); ++i)
		{
			//index the name of the object
			m_objectnames.push_back(i->name.GetString());

			// check if the member is a object
			if (i->value.IsObject())
			{
				for (auto j = i->value.MemberBegin(); j != i->value.MemberEnd(); ++j)
				{
					//index the components in the object to a single string
					componentTemp += j->name.GetString();
					componentTemp += ",";

					if (j->value.IsObject())
					{
						for (auto k = j->value.MemberBegin(); k != j->value.MemberEnd(); ++k)
						{
							//index variables in the components into a single string
							variableTemp += k->name.GetString();
							variableTemp += ",";

						}
						//add the string of variables to the vector of variables
						variableMap.insert({ std::make_pair(i->name.GetString(), j->name.GetString()), variableTemp });
						//clear the string
						variableTemp.clear();
					}

				}
				//add the string of components to the vector of components
				componentMap.insert({ i->name.GetString(), componentTemp });
				//clear the string
				componentTemp.clear();
			}

		}

	}

	std::string ObjectDeserializer::returnComponents(std::string object)
	{
		//return the components of the object
		return componentMap[object];
	}

	std::string ObjectDeserializer::returnVariables(std::string object, std::string component)
	{
		//return the variables of the component
		return variableMap[std::make_pair(object, component)];
	}

	void ObjectDeserializer::printContainers()
	{
		//print object vector
		std::cout << "Objects: " << std::endl;
		for (auto i = m_objectnames.begin(); i != m_objectnames.end(); ++i)
		{
			std::cout << *i << std::endl;
		}

		//print component map
		std::cout << "Components: " << std::endl;
		for (auto i = componentMap.begin(); i != componentMap.end(); ++i)
		{
			std::cout << i->first << " " << i->second << std::endl;
		}

		//print variable map
		std::cout << "Variables: " << std::endl;
		for (auto i = variableMap.begin(); i != variableMap.end(); ++i)
		{
			std::cout << i->first.first << " " << i->first.second << " " << i->second << std::endl;
		}
	}

	bool ObjectDeserializer::findObject(std::string name)
	{
		//find inside of the vector of objects
		auto it = std::find(m_objectnames.begin(), m_objectnames.end(), name);
		if (it != m_objectnames.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ObjectDeserializer::findComponent(std::string object, std::string component)
	{
		//find inside of the indexed unordered map
		if (componentMap.find(object) != componentMap.end())
		{
			//if the object exists, check if the component exists
			if (componentMap[object].find(component) != std::string::npos)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool ObjectDeserializer::findVariable(std::string object, std::string component, std::string variable)
	{
		//find inside of the indexed unordered map
		if (variableMap.find(std::make_pair(object, component)) != variableMap.end())
		{
			//if the object and component exists, check if the variable exists
			if (variableMap[std::make_pair(object, component)].find(variable) != std::string::npos)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	//getters
	std::vector < std::string > ObjectDeserializer::getObjectVector() const
	{
		return m_objectnames;
	}
	//getter
	std::unordered_map<std::string, std::string> ObjectDeserializer::getComponentMap() const
	{
		return componentMap;
	}

	// getter
	std::unordered_map < Pair, std::string, Pair_Hash> ObjectDeserializer::getVariableMap() const
	{
		return variableMap;
	}

	// lists all the calues tha are inside of the document
	std::vector<std::string> ObjectDeserializer::listValues()
	{
		std::vector<std::string> temp;
		for (auto i = m_document.MemberBegin(); i != m_document.MemberEnd(); ++i)
		{
			temp.push_back(i->name.GetString());
		}
		return temp;
	}

	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, int& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				value = m_document[objectname][variablename].GetInt();
				return true;
			}

			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//FLOAT
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, float& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				value = m_document[objectname][componentname].GetFloat();
				return true;
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//DOUBLE
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, double& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				value = m_document[objectname][variablename].GetDouble();
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//BOOL
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, bool& value)
	{
		//check if value exists
		if (m_document.HasMember(objectname))
		{
			//extract value and search for the component
			rapidjson::Value& temp = m_document[objectname];
			//search for component
			if (temp.HasMember(variablename))
			{
				//extract value
				rapidjson::Value& temp2 = temp[variablename];
				//convert from string to int
				value = std::stoi(temp2.GetString());
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist. " << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist." << std::endl;
			return false;
		}
	}

	//STRING
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::string& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				value = m_document[objectname][variablename].GetString();
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	/*******************************************************************************

						Object Value Retrival Functions

	 *******************************************************************************/

	 //INT
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, int& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					value = m_document[objectname][componentname][variablename].GetInt();
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//FLOAT
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, float& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					value = m_document[objectname][componentname][variablename].GetFloat();
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//DOUBLE
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, double& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					value = m_document[objectname][componentname][variablename].GetDouble();
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//BOOL
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, bool& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					value = m_document[objectname][componentname][variablename].GetBool();
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//STRING
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::string& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					value = m_document[objectname][componentname][variablename].GetString();
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	/*******************************************************************************

						Component Array Retrival Functions

	 *******************************************************************************/

	 //INT
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::vector<int>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				for (rapidjson::SizeType i = 0; i < m_document[objectname][variablename].Size(); i++)
				{
					value.push_back(m_document[objectname][variablename][i].GetInt());
				}
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//FLOAT
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::vector<float>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				for (rapidjson::SizeType i = 0; i < m_document[objectname][variablename].Size(); i++)
				{
					value.push_back(m_document[objectname][variablename][i].GetFloat());
				}
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//DOUBLE
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::vector<double>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				for (rapidjson::SizeType i = 0; i < m_document[objectname][variablename].Size(); i++)
				{
					value.push_back(m_document[objectname][variablename][i].GetDouble());
				}
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//BOOL
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::vector<bool>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				for (rapidjson::SizeType i = 0; i < m_document[objectname][variablename].Size(); i++)
				{
					value.push_back(m_document[objectname][variablename][i].GetBool());
				}
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//Deserialize Array string
	bool ObjectDeserializer::get_value(const char* objectname, const char* variablename, std::vector<std::string>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(variablename))
			{
				for (rapidjson::SizeType i = 0; i < m_document[objectname][variablename].Size(); i++)
				{
					value.push_back(m_document[objectname][variablename][i].GetString());
				}
				return true;
			}
			else
			{
				//std::cout << "Variable " << variablename << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	/*******************************************************************************

						Object Array Retrival Functions

	 *******************************************************************************/

	 //INT
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<int>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					for (rapidjson::SizeType i = 0; i < m_document[objectname][componentname][variablename].Size(); i++)
					{
						value.push_back(m_document[objectname][componentname][variablename][i].GetInt());
					}
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//FLOAT
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<float>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					for (rapidjson::SizeType i = 0; i < m_document[objectname][componentname][variablename].Size(); i++)
					{
						value.push_back(m_document[objectname][componentname][variablename][i].GetFloat());
					}
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//DOUBLE
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<double>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					for (rapidjson::SizeType i = 0; i < m_document[objectname][componentname][variablename].Size(); i++)
					{
						value.push_back(m_document[objectname][componentname][variablename][i].GetDouble());
					}
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//BOOL
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<bool>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					for (rapidjson::SizeType i = 0; i < m_document[objectname][componentname][variablename].Size(); i++)
					{
						value[i] = m_document[objectname][componentname][variablename][i].GetBool();
					}
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//STRING
	bool ObjectDeserializer::get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<std::string>& value) const
	{
		if (m_document.HasMember(objectname))
		{
			if (m_document[objectname].HasMember(componentname))
			{
				if (m_document[objectname][componentname].HasMember(variablename))
				{
					for (rapidjson::SizeType i = 0; i < m_document[objectname][componentname][variablename].Size(); i++)
					{
						value.push_back(m_document[objectname][componentname][variablename][i].GetString());
					}
					return true;
				}
				else
				{
					//std::cout << "Variable " << variablename << " does not exist in " << componentname << std::endl;
					return false;
				}
			}
			else
			{
				//std::cout << "Component " << componentname << " does not exist in " << objectname << std::endl;
				return false;
			}
		}
		else
		{
			//std::cout << "Object " << objectname << " does not exist" << std::endl;
			return false;
		}
	}

	//
	ObjectBuilder::ObjectBuilder()
	{
		m_value.SetObject();

	}

	ObjectBuilder::~ObjectBuilder()
	{
		m_value.SetNull();

	}

	//add a value to the value builder
	void ObjectBuilder::clearValue()
	{
		m_value.SetObject();
	}


	rapidjson::Value& ObjectBuilder::getValue()
	{
		return m_value;
	}

	void ObjectBuilder::setValue(rapidjson::Value& value)
	{
		m_value = value;
	}

	void ObjectBuilder::printValue()
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		m_value.Accept(writer);
		//std::cout << buffer.GetString() << std::endl;
	}

	void ObjectBuilder::insertComponent(rapidjson::Value& component, std::string name, Allocator allocator)
	{
		rapidjson::Value key(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, component, allocator);
	}

	/*******************************************************************************

							Insert variables into a component

	*******************************************************************************/

	// string type
	void ObjectBuilder::insertValue(std::string name, std::string value, Allocator allocator)
	{
		//create value
		rapidjson::Value temp, key;
		temp.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		//set key
		m_value.AddMember(key, temp, allocator);
	}

	// int type
	void ObjectBuilder::insertValue(std::string name, int value, Allocator allocator)
	{
		rapidjson::Value temp(value), key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		//set key
		m_value.AddMember(key, temp, allocator);
	}

	// float type
	void ObjectBuilder::insertValue(std::string name, float value, Allocator allocator)
	{
		rapidjson::Value temp(value), key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		//set key
		m_value.AddMember(key, temp, allocator);
	}

	// double type
	void ObjectBuilder::insertValue(std::string name, double value, Allocator allocator)
	{
		rapidjson::Value temp(value), key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		//set key
		m_value.AddMember(key, temp, allocator);
	}

	// bool type
	void ObjectBuilder::insertValue(std::string name, bool value, Allocator allocator)
	{
		rapidjson::Value temp(value), key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		//set key
		m_value.AddMember(key, temp, allocator);
	}

	/*******************************************************************************

							Insert Arrays into a component

	*******************************************************************************/

	// INT
	void ObjectBuilder::insertArray(std::string name, std::vector<int> value, Allocator allocator)
	{
		//set allocator
		rapidjson::Value temp;
		temp.SetArray();
		for (int i = 0; i < value.size(); i++)
		{
			rapidjson::Value temp2;
			temp2.SetInt(value[i]);
			temp.PushBack(temp2, allocator);
		}
		rapidjson::Value key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, temp, allocator);
	}

	// FLOAT
	void ObjectBuilder::insertArray(std::string name, std::vector<float> value, Allocator allocator)
	{
		//set allocator
		rapidjson::Value temp;
		temp.SetArray();
		for (int i = 0; i < value.size(); i++)
		{
			rapidjson::Value temp2;
			temp2.SetFloat(value[i]);
			temp.PushBack(temp2, allocator);
		}
		rapidjson::Value key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, temp, allocator);
	}

	// DOUBLE
	void ObjectBuilder::insertArray(std::string name, std::vector<double> value, Allocator allocator)
	{
		//set allocator
		rapidjson::Value temp;
		temp.SetArray();
		for (int i = 0; i < value.size(); i++)
		{
			rapidjson::Value temp2;
			temp2.SetDouble(value[i]);
			temp.PushBack(temp2, allocator);
		}
		rapidjson::Value key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, temp, allocator);
	}

	// BOOL
	void ObjectBuilder::insertArray(std::string name, std::vector<bool> value, Allocator allocator)
	{
		//set allocator
		rapidjson::Value temp;
		temp.SetArray();
		for (int i = 0; i < value.size(); i++)
		{
			rapidjson::Value temp2;
			temp2.SetBool(value[i]);
			temp.PushBack(temp2, allocator);
		}
		rapidjson::Value key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, temp, allocator);
	}

	// STRING
	void ObjectBuilder::insertArray(std::string name, std::vector<std::string> value, Allocator allocator)
	{
		//set allocator
		rapidjson::Value temp;
		temp.SetArray();
		for (int i = 0; i < value.size(); i++)
		{
			rapidjson::Value temp2;
			temp2.SetString(value[i].data(), static_cast<rapidjson::SizeType>(value[i].size()), allocator);
			temp.PushBack(temp2, allocator);
		}
		rapidjson::Value key;
		key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
		m_value.AddMember(key, temp, allocator);
	}
}
