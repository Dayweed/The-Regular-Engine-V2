#pragma once

// rapidJSON Includes
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"


// Standard Includes
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#pragma region OldSerializerIncludes
#include <unordered_map>

typedef std::pair<std::string, std::string > Pair;
typedef rapidjson::Document::AllocatorType& Allocator;
struct Pair_Hash
{
	template <class T1, class T2>
	std::size_t operator()(const std::pair<T1, T2>& pair) const
	{
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};
#pragma endregion

namespace TRE
{
	class ValueFactory
	{
	public:
		ValueFactory(); // Creates a empty new value
		ValueFactory(rapidjson::GenericValue<rapidjson::UTF8<>> value); // Copy Current values of a value
		~ValueFactory();

		// Get the value
		rapidjson::GenericValue<rapidjson::UTF8<>> GetValue();

		// Adding a object as a new member to the value
		void insertMember(std::string key, rapidjson::GenericValue<rapidjson::UTF8<>> value);
		
		/* !
		@function  insertValue
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function inserts raw data into the value. Handles checking if the data currently already exists in the value.

		*//*__________________________________________________________________________*/
		template<typename T>
		void insertValue(std::string key, T value);
		
		
	private:
		rapidjson::GenericValue<rapidjson::UTF8<>> m_Value;
	};

	
	class Serializer
	{
	public:
		Serializer();
		~Serializer();
		
		bool ReadFromFile(std::string filepath);
		bool WriteToFile(std::string filepath);
		

	private:
		rapidjson::GenericDocument<rapidjson::UTF8<>> m_Document;
		rapidjson::GenericValue<rapidjson::UTF8<>> m_Value;
		
	};

	class Deserializer
	{
		
	};

#pragma region OldSerializer
	/* !
	@function  ReadExternalFile
	@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

	@params	   rapidjson::Document& document - This is the object to store the JSON file
	@params	   const char* filename - This is the file to read from.

	@brief	   This function currently has a preset folder path that it is using currently.
			   This is because all the JSON files are currently stored in the same folder.
			   the filepathing will change the moment the filesystem and content browser is
			   implemented.

	*//*__________________________________________________________________________*/
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

	/* !
	@function  WrtieToExternalFile
	@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

	@params	   rapidjson::Document& document - This is the object read from for the write operation
	@params	   const char* filename - This is the file to write to .

	@brief	   This function does a write operation to an external JSON file.The contents
			   the contents of the JSON file that it is writting to will be wiped and replaced.
			   The file that it is currently writing to is all placed in a fixed folder path.
			   This is because all the JSON files are currently stored in the same folder.
			   the filepathing will change the moment the filesystem and content browser is
			   implemented.

	*//*__________________________________________________________________________*/
	void WriteToExternalFile(rapidjson::Document& document, const char* filename)
	{
		std::string folderpath = "../";
		std::string filepath = folderpath + filename;
		std::ofstream file(filepath);

		rapidjson::OStreamWrapper osw(file);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		document.Accept(writer);
	}

	/* !
	@class ObjectSerializer
	@author    Tan Wee Yi (weeyi.t@digipen.edu)
	@brief This class contains functions that will aid with the serialization of complex
		data structures into a JSON file.

	*//*__________________________________________________________________________*/
	class ObjectSeralizer
	{
	public:

		// constructor
		ObjectSeralizer()
		{
			m_document.SetObject();
		}

		//constructor with filename
		ObjectSeralizer(const char* filename)
		{
			m_document.SetObject();
			ReadExternalFile(m_document, filename);
		}
		// destructor
		virtual ~ObjectSeralizer()
		{
			m_map.~vector();
			m_array.~vector();
			m_object.~vector();
			m_document.SetNull();
		}


		/* !
		@function  UpdateObjectInDocument
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Below are overloaded functions that allows access into the document for editing the values.

		*//*__________________________________________________________________________*/
		//INT
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, int value)
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

		//FLOAT
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, float value)
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

		//double
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, double value)
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

		//STRING
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, std::string value)
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

		//BOOL
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, bool value)
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

		// Updating to the external JSON file
		void writeToDoc(const char* filename)
		{
			WriteToExternalFile(m_document, filename);
		}

		/* !
		@function  AddObjectToDoc
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu)

		@brief Function adds a rapidjson value that contains an object into the document

		*//*__________________________________________________________________________*/
		void AddObjectToDoc(rapidjson::Value& v_value, const char* name)
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


		rapidjson::Document& getDoc()
		{
			return m_document;
		}

	private:
		rapidjson::Document m_document;
		std::vector < std::pair < std::string, std::string >> m_map;
		std::vector < std::pair < std::string, std::string >> m_array;
		std::vector < std::pair < std::string, std::string >> m_object;


	};

	/* !
	@class  ObjectDeserializer
	@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

	@brief Class aids with the deserialization of the JSON file and allows access to the
		that is stored in it.

	*//*__________________________________________________________________________*/
	class ObjectDeserializer
	{

	public:

		//constructor
		ObjectDeserializer()
		{
			m_document.SetObject();
		}

		//constructor with filename
		ObjectDeserializer(const char* filename)
		{
			ReadExternalFile(m_document, filename);
			indexDocument();
		}

		//destructor
		~ObjectDeserializer()
		{
			m_document.SetNull();
			m_objectnames.clear();
			componentMap.clear();
			variableMap.clear();
		}

		//Load file Manually
		void loadFile(const char* filename)
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

		// loading the document with a string that is typed with JSON indexing in mind
		void loadString(const char* string)
		{
			//parse string
			m_document.Parse(string);
		}

		// loading a document into the deserializer from another document
		void loadDocument(rapidjson::Document doc)
		{
			//copy document
			m_document.CopyFrom(doc, m_document.GetAllocator());
		}

		// loading a value into the document
		void loadValue(rapidjson::Value val)
		{
			//copy value
			m_document.CopyFrom(val, m_document.GetAllocator());
		}

		//loading a value into the document
		void loadValue(rapidjson::Value* val)
		{
			//copy value
			m_document.CopyFrom(*val, m_document.GetAllocator());
		}


		/* !
		@function  checkComponentIfObject
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function is responsible to check if the object is a object or just a variable

		*//*__________________________________________________________________________*/
		bool checkComponentIfObject(std::string object, std::string name)
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

		/* !
		@function  indexDocument
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function will index the document into containers so that the users can find out what
		data is stored in the documents

		*//*__________________________________________________________________________*/
		void indexDocument()
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

		// return component container
		std::string returnComponents(std::string object)
		{
			//return the components of the object
			return componentMap[object];
		}

		// return variable container
		std::string returnVariables(std::string object, std::string component)
		{
			//return the variables of the component
			return variableMap[std::make_pair(object, component)];
		}

		// print out all the contents in the container
		void printContainers()
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

		// find and see if the object exists in the document
		bool findObject(std::string name)
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

		// find and see if the component exists in the document
		bool findComponent(std::string object, std::string component)
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

		// find and see if the variable of the object and component exists in the document
		bool findVariable(std::string object, std::string component, std::string variable)
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
		std::vector < std::string > getObjectVector() const
		{
			return m_objectnames;
		}
		//getter
		std::unordered_map<std::string, std::string> getComponentMap() const
		{
			return componentMap;
		}

		// getter
		std::unordered_map < Pair, std::string, Pair_Hash> getVariableMap() const
		{
			return variableMap;
		}

		// lists all the calues tha are inside of the document
		std::vector<std::string> listValues()
		{
			std::vector<std::string> temp;
			for (auto i = m_document.MemberBegin(); i != m_document.MemberEnd(); ++i)
			{
				temp.push_back(i->name.GetString());
			}
			return temp;
		}

		/* !
		@function  get_value
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief The overloaded functions below allows for users to access the values
			of the variables in the doucment. It has been accounted for the use in the current engine

		*//*__________________________________________________________________________*/

		/*******************************************************************************

							Component Value Retrival Functions

		 *******************************************************************************/

		 //INT
		bool get_value(const char* objectname, const char* variablename, int& value) const
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
		bool get_value(const char* objectname, const char* componentname, float& value) const
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
		bool get_value(const char* objectname, const char* variablename, double& value) const
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
		bool get_value(const char* objectname, const char* variablename, bool& value)
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
		bool get_value(const char* objectname, const char* variablename, std::string& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, int& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, float& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, double& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, bool& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::string& value) const
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
		bool get_value(const char* objectname, const char* variablename, std::vector<int>& value) const
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
		bool get_value(const char* objectname, const char* variablename, std::vector<float>& value) const
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
		bool get_value(const char* objectname, const char* variablename, std::vector<double>& value) const
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
		bool get_value(const char* objectname, const char* variablename, std::vector<bool>& value) const
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
		bool get_value(const char* objectname, const char* variablename, std::vector<std::string>& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<int>& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<float>& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<double>& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<bool>& value) const
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
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<std::string>& value) const
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

	private:

		rapidjson::Document m_document;
		std::vector<std::string> m_objectnames;
		std::unordered_map < std::string, std::string > componentMap;
		std::unordered_map < Pair, std::string, Pair_Hash>  variableMap;

	};

	/* !
		@class  ObjectBuilder
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief This class is responsible for allowing users to be able to build a component/ object
			as they want it to be before inserting the value into a document.

		*//*__________________________________________________________________________*/
	class ObjectBuilder
	{
	public:

		ObjectBuilder()
		{
			m_value.SetObject();

		}

		~ObjectBuilder()
		{
			m_value.SetNull();

		}

		//add a value to the value builder
		void clearValue()
		{
			m_value.SetObject();
		}


		rapidjson::Value& getValue()
		{
			return m_value;
		}

		void setValue(rapidjson::Value& value)
		{
			m_value = value;
		}

		void printValue()
		{
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			m_value.Accept(writer);
			//std::cout << buffer.GetString() << std::endl;
		}
		/* !
		@function  insert_component
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Functions below allows for the building of custom components and objects
			by allowing users to insert values into the component/ object.

		*//*__________________________________________________________________________*/

		//inserts another component into the current value
		void insertComponent(rapidjson::Value& component, std::string name, Allocator allocator)
		{
			rapidjson::Value key(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			m_value.AddMember(key, component, allocator);
		}

		/* !
		@function  insertValue
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief TFunction allow for the adding of variables into a value type object in
			rapidjson

		*//*__________________________________________________________________________*/

		/*******************************************************************************

							Insert variables into a component

		 *******************************************************************************/

		 // string type
		void insertValue(std::string name, std::string value, Allocator allocator)
		{
			//create value
			rapidjson::Value temp, key;
			temp.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
			key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			//set key
			m_value.AddMember(key, temp, allocator);
		}

		// int type
		void insertValue(std::string name, int value, Allocator allocator)
		{
			rapidjson::Value temp(value), key;
			key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			//set key
			m_value.AddMember(key, temp, allocator);
		}

		// float type
		void insertValue(std::string name, float value, Allocator allocator)
		{
			rapidjson::Value temp(value), key;
			key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			//set key
			m_value.AddMember(key, temp, allocator);
		}

		// double type
		void insertValue(std::string name, double value, Allocator allocator)
		{
			rapidjson::Value temp(value), key;
			key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			//set key
			m_value.AddMember(key, temp, allocator);
		}

		// bool type
		void insertValue(std::string name, bool value, Allocator allocator)
		{
			rapidjson::Value temp(value), key;
			key.SetString(name.data(), static_cast<rapidjson::SizeType>(name.size()), allocator);
			//set key
			m_value.AddMember(key, temp, allocator);
		}

		/* !
		@function  insertArray
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function inserts arrays into the value object

		*//*__________________________________________________________________________*/

		/*******************************************************************************

							Insert Arrays into a component

		 *******************************************************************************/

		 // INT
		void insertArray(std::string name, std::vector<int> value, Allocator allocator)
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
		void insertArray(std::string name, std::vector<float> value, Allocator allocator)
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
		void insertArray(std::string name, std::vector<double> value, Allocator allocator)
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
		void insertArray(std::string name, std::vector<bool> value, Allocator allocator)
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
		void insertArray(std::string name, std::vector<std::string> value, Allocator allocator)
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

	private:

		rapidjson::Value m_value;

	};

#pragma endregion
}
