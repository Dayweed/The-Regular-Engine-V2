#include "Serialization.h"

namespace TRE
{
	ValueFactory::ValueFactory()
	{
		m_Value.SetObject();
		m_Document.SetObject();
	}
	
	ValueFactory::ValueFactory(rapidjson::GenericValue<rapidjson::UTF8<>> value)
	{
		m_Value.CopyFrom(value, m_Document.GetAllocator());
	}

	void ValueFactory::LoadDocument(std::string filename)
	{
		
	}
}
