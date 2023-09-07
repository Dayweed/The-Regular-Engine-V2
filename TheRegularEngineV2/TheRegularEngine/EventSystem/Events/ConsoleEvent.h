#pragma once
#include "EventBase.h"
#include <string>


namespace TRE
{
	struct ConsoleDebugEvent : Event
	{
		std::string m_Msg;
		ConsoleDebugEvent(std::string&& msg) : m_Msg(std::move(msg)) {}
	};
}