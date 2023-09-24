#pragma once
#include "EventBase.h"
#include <string>
#include <chrono>

namespace TRE
{
	struct ConsoleDebugEvent : Event
	{
		std::string m_Msg;
		ConsoleDebugEvent(std::string&& msg) : m_Msg(std::move(msg)) {}
	};
	//To use the event just include this file and publish the event like this:
	//EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Testing Key inputs here" });

	enum class TimerType :int
	{
		VKRENDER,
		PHYSICS,
		SCRIPTS,
		MAX
	};

	struct SendTimeTakenEvent : Event
	{
		std::chrono::nanoseconds m_ms{};
		TimerType m_type{};
		SendTimeTakenEvent() = delete;
		SendTimeTakenEvent(const std::chrono::nanoseconds& ms, TimerType type) : m_ms(ms), m_type(type) {}
	};
}
