#include "pch.h"

#include"Scripting/ScriptingSystem.h"

#include "Scripting/ScriptEngine.h"

namespace TRE
{
	void ScriptingSystem::Init()
	{
		
	}

	void ScriptingSystem::Update()
	{
		if(DemoInit == true)
		{
			//ScriptEngine::DemoInit();
			DemoInit = false;
		}
		
	}

	void ScriptingSystem::GameUpdate()
	{
		//ScriptEngine::DemoUpdate();
	}

	void ScriptingSystem::LateUpdate()
	{
	}

	void ScriptingSystem::BeforeReset()
	{
	}

	void ScriptingSystem::OnDestroyEntities()
	{
	}

	void ScriptingSystem::Shutdown()
	{
	}


}
