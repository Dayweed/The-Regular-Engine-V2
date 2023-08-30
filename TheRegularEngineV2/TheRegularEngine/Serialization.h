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
}
