#pragma once
#include "EventBase.h"
#include "TREIncludes.h"
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

	struct SendTimeTakenEvent : Event
	{
		std::unordered_map<std::string, Timer*> m_Timers{};
		SendTimeTakenEvent() = delete;
		SendTimeTakenEvent(std::unordered_map<std::string, Timer*> timer) : m_Timers(timer) {}
	};

	struct ToggleRunEvent : Event
	{
		bool m_Playing{ false };
		ToggleRunEvent() = delete;
		ToggleRunEvent(bool playing) : m_Playing(playing) {}
	};

	struct ResetSceneEvent : Event
	{
		bool m_Nth{ false };
		ResetSceneEvent() = delete;
		ResetSceneEvent(bool nth) : m_Nth(nth) {}
	};

	struct GridAndSnapEvent : Event
	{
		float m_PosIncreament{};
		float m_RotIncreament{};
		float m_ScaleIncreament{};
		GridAndSnapEvent() = delete;
		GridAndSnapEvent(float pos, float rot, float scale) : m_PosIncreament(pos), m_RotIncreament(rot), m_ScaleIncreament(scale) {}
	};
}
