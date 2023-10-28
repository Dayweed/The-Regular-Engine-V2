#include "pch.h"

#include"Scripting/ScriptingSystem.h"
#include"Scripting/ScriptComponent.h"
#include "Scripting/ScriptEngine.h"

namespace TRE
{
	void ScriptingSystem::Init()
	{
		m_IsRunning = true;
		m_ScriptableUpdate = true;
	}

	void ScriptingSystem::Update()
	{
		if(m_ScriptableUpdate == true)
		{
			m_ScriptableUpdate = false;
		}

		if(m_IsRunning == true)
		{
			// here
		}

		UpdateScriptableObjects();
		CheckForNewScriptableObjects();
	}

	void ScriptingSystem::GameUpdate()
	{
		if(m_IsRunning == true)
		{
			//inital Create entity instances
			for(auto e: m_ScriptEntities)
			{
				ScriptEngine::OnCreateEntity(e);
			}
			m_IsRunning = false;
			
		}

		for(auto e: m_ScriptEntities)
		{
			ScriptEngine::OnUpdateEntity(e);
		}

		ScriptEngine::UpdateScriptingMain();
		
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

	void ScriptingSystem::AddScriptableObject(Entity entity)
	{
		//interate through the vector and add the entity while ensuring no duplicates
		for(auto e : m_ScriptEntities)
		{
			if(e == entity)
			{
				return;
			}
		}
		// if the entity is not in the vector, add it
		m_ScriptEntities.push_back(entity);
	}

	void ScriptingSystem::InitializeScriptableObjects()
	{
		std::vector<Entity> temp = ECSManager::Instance().GetAllEntities();
		for(auto e: temp)
		{
			if(ECSManager::Instance().EntityHasComponent<ScriptComponent>(e))
			{
				AddScriptableObject(e);
			}
		}
	}

	void ScriptingSystem::RemoveScriptableObject(Entity entity)
	{
		//interate through the vector and remove the entity
		for(auto e : m_ScriptEntities)
		{
			if(e == entity)
			{
				erase(m_ScriptEntities, e);
				return;
			}
		}
	}

	void ScriptingSystem::UpdateScriptableObjects()
	{
		// iterate through the vector and update the scriptable objects
		for(auto e: m_ScriptEntities)
		{
			if(e->GetComponent<ScriptComponent>().m_IsDirty)
			{
				// update the scriptable objec
			}
		}
	}

	void ScriptingSystem::CheckForNewScriptableObjects()
	{
		// check if there are any new scriptable objects
		std::vector<Entity> temp = ECSManager::Instance().GetAllEntities();
		for(auto e: temp)
		{
			if(ECSManager::Instance().EntityHasComponent<ScriptComponent>(e))
			{
				AddScriptableObject(e);
			}
		}
	}


}
