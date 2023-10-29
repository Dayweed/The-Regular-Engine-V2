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

		CheckForNewScriptableObjects();
		UpdateScriptableObjects();
	}

	void ScriptingSystem::GameUpdate()
	{
		if(m_IsRunning == true)
		{
			//inital Create entity instances (only works if they are created before scene starts)
			for(auto e: m_ScriptEntities)
			{
				ScriptEngine::OnCreateEntity(e);
			}
			m_IsRunning = false;
			
		}

		// For Scripts just created
		for(auto e: m_ScriptEntities)
		{
			ScriptComponent& script{ e->GetComponent<ScriptComponent>() };
			if (script.m_RanStart) continue;

			ScriptEngine::OnStartEntity(e);
			script.m_RanStart = true;
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
		// Clear in case some was just added
		m_ScriptEntities.clear();
	}

	void ScriptingSystem::AfterReset()
	{
		// Add back all entities with Scripting
		InitializeScriptableObjects();
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
		m_ScriptEntities.clear();
		m_ScriptEntities = ECSManager::Instance().GetEntities<ScriptComponent>(true);

		//std::vector<Entity> temp = ECSManager::Instance().GetAllEntities();
		//for(auto e: temp)
		//{
		//	if(ECSManager::Instance().EntityHasComponent<ScriptComponent>(e))
		//	{
		//		AddScriptableObject(e);
		//	}

		//	for (int x = 0; x < m_ScriptEntities.size(); x++)
		//	{
		//		//Remove script entity from scripting system since no script component
		//		if (ECSManager::Instance().EntityHasComponent<ScriptComponent>(m_ScriptEntities[x]) == false)
		//		{
		//			if (m_ScriptEntities[x] == e)
		//				m_ScriptEntities.erase(m_ScriptEntities.begin() + x);
		//		}
		//	}
		//}
	}


}
