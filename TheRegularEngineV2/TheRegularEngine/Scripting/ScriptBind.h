#pragma once

#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/InputEvent.h"

namespace TRE
{
	class ScriptBind
	{
	public :
		static void RegisterComponents();
		static void RegisterFunctions();
	};

	class ScriptInputHandler
	{
	public:
		static ScriptInputHandler& Instance();
		void Update();

		void GetKeyPressed(const InputEvent& event);

		int GetKey() { return _key; }
		int GetState() { return _state; }
		void ResetSystem() { _key = 0; _state = 0; }

	private:

		ScriptInputHandler() {EventHandler::getEventHandlerInstance().subscribe(this, &ScriptInputHandler::GetKeyPressed);};
		ScriptInputHandler(ScriptInputHandler const&) = delete;
		void operator=(ScriptInputHandler const&) = delete;
		void* operator new(size_t) = delete;

		int _key{}, _state{};
		friend class ScriptEngine;
	};
}