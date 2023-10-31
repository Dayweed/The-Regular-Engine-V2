#include "pch.h"

#include "TREIncludes.h"

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

		CheckForNewScriptableObjects();
		UpdateScriptableObjects();
	}

	void ScriptingSystem::GameUpdate()
	{
		if(m_IsRunning == true)
		{	
			ScriptEngine::ReloadAssembly();
			//inital Create entity instances (only works if they are created before scene starts)
			for(auto e: m_ScriptEntities)
			{
				ScriptEngine::OnCreateEntity(e);
			}
			m_IsRunning = false;
			
		}

		// On Enable
		for (auto e : m_ScriptEntities)
		{
			Properties& prop{ e->GetComponent<Properties>() };
			if (prop.m_IsDirty && prop.m_Active)
			{
				e->GetComponent<Properties>().m_IsDirty = false;
				ScriptEngine::OnEnableEntity(e);
			}
		}

		// On Disable
		for (auto e : m_ScriptEntities)
		{
			Properties& prop{ e->GetComponent<Properties>() };
			if (prop.m_IsDirty && !prop.m_Active)
			{
				e->GetComponent<Properties>().m_IsDirty = false;
				ScriptEngine::OnDisableEntity(e);
			}
		}

		// For Scripts just created
		for(auto e: m_ScriptEntities)
		{
			ScriptComponent& script{ e->GetComponent<ScriptComponent>() };
			if (script.m_RanStart) continue;

			ScriptEngine::OnStartEntity(e);
			script.m_RanStart = true;
		}

		// Check for trigger
		std::vector<std::pair<Entity, Entity>> triggerEntries{ ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetTriggerHistory() };
		for (auto e : triggerEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerStay(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerStay(e.second, e.first);
		}

		// Check for collision
		std::vector<std::pair<Entity, Entity>> collisionEntries{ ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetCollisionHistory() };
		for (auto e : collisionEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionStay(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionStay(e.second, e.first);
		}

		// Update
		for(auto e: m_ScriptEntities)
		{
			ScriptEngine::OnUpdateEntity(e);
		}

		// Late Update
		for(auto e: m_ScriptEntities)
		{
			ScriptEngine::OnLateUpdateEntity(e);
		}

		// On Destroy
		for (auto e : m_ScriptEntities)
		{
			if (e->HasComponent<Removal>())
				ScriptEngine::OnDestroyEntity(e);
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
		m_IsRunning = true;
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
		//std::vector<Entity> temp = ECSManager::Instance().GetAllEntities(true);
		//for(auto e: temp)
		//{
		//	if(ECSManager::Instance().EntityHasComponent<ScriptComponent>(e))
		//	{
		//		AddScriptableObject(e);
		//		//ScriptEngine::CreateCSEntityData(e);
		//	}
		//}

		m_ScriptEntities.clear();
		m_ScriptEntities = ECSManager::Instance().GetEntities<ScriptComponent>(true);

		
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
			if(ECSManager::Instance().IsValidEntity(e) && e->GetComponent<ScriptComponent>().m_IsDirty)
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
