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
#include <cstdarg>

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
	void ReadExternalFile(rapidjson::Document& document, const char* filename);

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
	void WriteToExternalFile(rapidjson::Document& document, const char* filename);

	/* !
	@class ObjectSerializer
	@author    Tan Wee Yi (weeyi.t@digipen.edu)
	@brief This class contains functions that will aid with the serialization of complex
		data structures into a JSON file.

	*//*__________________________________________________________________________*/
	class ObjectSerializer
	{
	public:

		// constructor
		ObjectSerializer();

		//constructor with filename
		ObjectSerializer(const char* filename);

		// destructor
		virtual ~ObjectSerializer();


		/* !
		@function  UpdateObjectInDocument
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Below are overloaded functions that allows access into the document for editing the values.

		*//*__________________________________________________________________________*/
		//INT
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, int value);

		//FLOAT
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, float value);

		//double
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, double value);

		//STRING
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, std::string value);

		//BOOL
		bool updateObjectInDocument(const char* object, const char* component, const char* variable, bool value);

		// Updating to the external JSON file
		void writeToDoc(const char* filename);

		/* !
		@function  AddObjectToDoc
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu)

		@brief Function adds a rapidjson value that contains an object into the document

		*//*__________________________________________________________________________*/
		void AddObjectToDoc(rapidjson::Value& v_value, const char* name);


		rapidjson::Document& getDoc();

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
		ObjectDeserializer();

		//constructor with filename
		ObjectDeserializer(const char* filename);
		//destructor
		~ObjectDeserializer();

		//Load file Manually
		void loadFile(const char* filename);

		// loading the document with a string that is typed with JSON indexing in mind
		void loadString(const char* string);

		// loading a document into the deserializer from another document
		void loadDocument(rapidjson::Document doc);

		// loading a value into the document
		void loadValue(rapidjson::Value val);

		//loading a value into the document
		void loadValue(rapidjson::Value* val);


		/* !
		@function  checkComponentIfObject
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function is responsible to check if the object is a object or just a variable

		*//*__________________________________________________________________________*/
		bool checkComponentIfObject(std::string object, std::string name);

		/* !
		@function  indexDocument
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function will index the document into containers so that the users can find out what
		data is stored in the documents

		*//*__________________________________________________________________________*/
		void indexDocument();

		// return component container
		std::string returnComponents(std::string object);

		// return variable container
		std::string returnVariables(std::string object, std::string component);

		// print out all the contents in the container
		void printContainers();

		// find and see if the object exists in the document
		bool findObject(std::string name);

		// find and see if the component exists in the document
		bool findComponent(std::string object, std::string component);

		// find and see if the variable of the object and component exists in the document
		bool findVariable(std::string object, std::string component, std::string variable);

		//getters
		std::vector < std::string > getObjectVector() const;

		//getter
		std::unordered_map<std::string, std::string> getComponentMap() const;

		// getter
		std::unordered_map < Pair, std::string, Pair_Hash> getVariableMap() const;

		// lists all the calues tha are inside of the document
		std::vector<std::string> listValues();

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
		bool get_value(const char* objectname, const char* variablename, int& value) const;

		//FLOAT
		bool get_value(const char* objectname, const char* componentname, float& value) const;

		//DOUBLE
		bool get_value(const char* objectname, const char* variablename, double& value) const;

		//BOOL
		bool get_value(const char* objectname, const char* variablename, bool& value);

		//STRING
		bool get_value(const char* objectname, const char* variablename, std::string& value) const;

		/*******************************************************************************

							Object Value Retrival Functions

		 *******************************************************************************/

		 //INT
		bool get_value(const char* objectname, const char* componentname, const char* variablename, int& value) const;

		//FLOAT
		bool get_value(const char* objectname, const char* componentname, const char* variablename, float& value) const;

		//DOUBLE
		bool get_value(const char* objectname, const char* componentname, const char* variablename, double& value) const;

		//BOOL
		bool get_value(const char* objectname, const char* componentname, const char* variablename, bool& value) const;

		//STRING
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::string& value) const;

		/*******************************************************************************

							Component Array Retrival Functions

		 *******************************************************************************/

		 //INT
		bool get_value(const char* objectname, const char* variablename, std::vector<int>& value) const;

		//FLOAT
		bool get_value(const char* objectname, const char* variablename, std::vector<float>& value) const;

		//DOUBLE
		bool get_value(const char* objectname, const char* variablename, std::vector<double>& value) const;

		//BOOL
		bool get_value(const char* objectname, const char* variablename, std::vector<bool>& value) const;

		//Deserialize Array string
		bool get_value(const char* objectname, const char* variablename, std::vector<std::string>& value) const;

		/*******************************************************************************

							Object Array Retrival Functions

		 *******************************************************************************/

		 //INT
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<int>& value) const;

		//FLOAT
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<float>& value) const;

		//DOUBLE
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<double>& value) const;

		//BOOL
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<bool>& value) const;

		//STRING
		bool get_value(const char* objectname, const char* componentname, const char* variablename, std::vector<std::string>& value) const;

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

		ObjectBuilder();

		~ObjectBuilder();

		//add a value to the value builder
		void clearValue();

		rapidjson::Value& getValue();

		void setValue(rapidjson::Value& value);

		void printValue();

		/* !
		@function  insert_component
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Functions below allows for the building of custom components and objects
			by allowing users to insert values into the component/ object.

		*//*__________________________________________________________________________*/

		//inserts another component into the current value
		void insertComponent(rapidjson::Value& component, std::string name, Allocator allocator);

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
		void insertValue(std::string name, std::string value, Allocator allocator);

		// int type
		void insertValue(std::string name, int value, Allocator allocator);

		// float type
		void insertValue(std::string name, float value, Allocator allocator);

		// double type
		void insertValue(std::string name, double value, Allocator allocator);

		// bool type
		void insertValue(std::string name, bool value, Allocator allocator);

		/* !
		@function  insertArray
		@author    Tan Wee Yi  ( weeyi.t@digipen.edu )

		@brief Function inserts arrays into the value object

		*//*__________________________________________________________________________*/

		/*******************************************************************************

							Insert Arrays into a component

		 *******************************************************************************/

		 // INT
		void insertArray(std::string name, std::vector<int> value, Allocator allocator);

		// FLOAT
		void insertArray(std::string name, std::vector<float> value, Allocator allocator);

		// DOUBLE
		void insertArray(std::string name, std::vector<double> value, Allocator allocator);

		// BOOL
		void insertArray(std::string name, std::vector<bool> value, Allocator allocator);

		// STRING
		void insertArray(std::string name, std::vector<std::string> value, Allocator allocator);

	private:

		rapidjson::Value m_value;

	};

#pragma endregion
}
