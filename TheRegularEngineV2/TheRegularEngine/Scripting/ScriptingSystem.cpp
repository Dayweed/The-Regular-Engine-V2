#include "pch.h"

#include "TREIncludes.h"

#include"Scripting/ScriptingSystem.h"

#include "../../TheRegularEditor/src/ToolBarPanel.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"
#include"Scripting/ScriptComponent.h"
#include "Scripting/ScriptEngine.h"

namespace TRE
{
	void ScriptingSystem::Init()
	{
		m_IsRunning = true;
		m_ScriptableUpdate = true;

		EventHandler::getEventHandlerInstance().subscribe(this, &ScriptingSystem::CallRecompile);
	}

	void ScriptingSystem::Update()
	{
		if (m_ScriptableUpdate == true)
		{
			m_ScriptableUpdate = false;
		}

		CheckForNewScriptableObjects();
		UpdateScriptableObjects();
		//ScriptEngine::UpdateScriptingMain();
	}

	void ScriptingSystem::GameUpdate()
	{

		if (m_IsRunning == true)
		{
			//inital Create entity instances (only works if they are created before scene starts)
			for (auto e : m_ScriptEntities)
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
		for (auto e : m_ScriptEntities)
		{
			ScriptComponent& script{ e->GetComponent<ScriptComponent>() };
			if (script.m_RanStart) continue;

			ScriptEngine::OnStartEntity(e);
			script.m_RanStart = true;
		}

		// Check for trigger
		// Enter
		std::vector<std::pair<Entity, Entity>> triggerEnterEntries;
		std::vector<std::pair<Entity, Entity>> triggerStayEntries;
		std::vector<std::pair<Entity, Entity>> triggerExitEntries;
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetTriggerHistory(triggerEnterEntries, triggerStayEntries, triggerExitEntries);
		for (auto e : triggerEnterEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerEnter(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerEnter(e.second, e.first);
		}
		//Stay
		for (auto e : triggerStayEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerStay(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerStay(e.second, e.first);
		}
		// Exit
		for (auto e : triggerExitEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerExit(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnTriggerExit(e.second, e.first);
		}

		// Check for collision
		// Enter
		std::vector<std::pair<Entity, Entity>> collisionEnterEntries;
		std::vector<std::pair<Entity, Entity>> collisionStayEntries;
		std::vector<std::pair<Entity, Entity>> collisionExitEntries;
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetCollisionHistory(collisionEnterEntries, collisionStayEntries, collisionExitEntries);
		for (auto e : collisionEnterEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionEnter(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionEnter(e.second, e.first);
		}
		// Stay
		for (auto e : collisionStayEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionStay(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionStay(e.second, e.first);
		}
		// Exit
		for (auto e : collisionExitEntries)
		{
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.first) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionExit(e.first, e.second);
			if (std::find(m_ScriptEntities.begin(), m_ScriptEntities.end(), e.second) != m_ScriptEntities.end())
				ScriptEngine::OnCollisionExit(e.second, e.first);
		}

		// Update
		for (size_t i{}; i < m_ScriptEntities.size(); ++i)
		{
			ScriptEngine::OnUpdateEntity(m_ScriptEntities[i]);
		}

		// Late Update
		for (size_t i{}; i < m_ScriptEntities.size(); ++i)
		{
			ScriptEngine::OnLateUpdateEntity(m_ScriptEntities[i]);
		}

		// On Destroy
		for (size_t i{}; i < m_ScriptEntities.size(); ++i)
		{
			if (m_ScriptEntities[i]->HasComponent<Removal>())
				ScriptEngine::OnDestroyEntity(m_ScriptEntities[i]);
		}

		//ScriptEngine::UpdateScriptingMain();

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
		for (auto e : m_ScriptEntities)
		{
			if (e == entity)
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
		for (auto e : m_ScriptEntities)
		{
			if (e == entity)
			{
				erase(m_ScriptEntities, e);
				return;
			}
		}
	}

	void ScriptingSystem::UpdateScriptableObjects()
	{
		// iterate through the vector and update the scriptable objects
		for (auto e : m_ScriptEntities)
		{
			if (ECSManager::Instance().IsValidEntity(e) && e->GetComponent<ScriptComponent>().m_IsDirty)
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

	void ScriptingSystem::CallRecompile(const ToggleRunEvent& event)
	{
		//Hot reload only when not game mode
#ifdef WINDOWED
		if (event.m_Playing == true)
		{
			ScriptEngine::RecompileScripts();
			ScriptEngine::ReloadAssembly();
		}
#else
#endif
	}
}
