#include "pch.h"
#include "Engine.h"
#include "Scripting/ScriptEngine.h"

#define HAHA_PHYSICS 1
#if HAHA_PHYSICS
#include "PhysX/PxPhysicsAPI.h"
#endif

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
		ScriptEngine::InitMono();

		#if HAHA_PHYSICS
		using namespace physx;
		PxDefaultErrorCallback gErrorCallback;
		PxDefaultAllocator gAllocator;
		PxFoundation* gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		#endif
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