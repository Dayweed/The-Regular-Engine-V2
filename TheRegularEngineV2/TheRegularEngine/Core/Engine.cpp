#include "pch.h"
#include "Engine.h"
#include "Scripting/ScriptEngine.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
		ScriptEngine::InitMono();
	}

	Engine::~Engine()
	{

	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();
		}
	}

	void Engine::Shutdown()
	{

	}
}