#include "Serialization.h"

namespace TRE
{
	ValueFactory::ValueFactory()
	{
		m_Value.SetObject();
	}
	
	ValueFactory::ValueFactory(rapidjson::GenericValue<rapidjson::UTF8<>> value)
	{
		m_Value.CopyFrom(value, );
	}
}
