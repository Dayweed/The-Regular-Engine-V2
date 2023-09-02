/*!
	@file    MemoryManager.cpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    14/09/2022
	@brief   This file handles all new and delete related to ecs and checks for
			 mem leaks

	Copyright (C) 2022 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

#include "pch.h"
#include "MemoryManager.h"

namespace TRE
{
	/* !
	@function	Instance
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params

	@brief		Creates a static instance of the MemoryManager

	*//*__________________________________________________________________________*/
	MemoryManager& MemoryManager::Instance()
	{
		static MemoryManager instance{};
		return instance;
	}

	Entity MemoryManager::GetUndeployedEntity()
	{
		// Allocate additional space if there is no undeployed
		if (m_UndeployedEntityList.empty())
		{
			AllocateEntitySize(m_AllEntityList.size());
		}

		// Get an Entity_ID from Undeployed and assign it to Deployed, returns the Entity
		Entity_ID id{ *m_UndeployedEntityList.begin() };
		m_UndeployedEntityList.erase(id);
		m_DeployedEntityList.emplace(id);
		m_AllEntityList[id]->RemoveComponent<Undeployed>();
		return m_AllEntityList[id];
	}

	void MemoryManager::ReleaseDeployedEntity(Entity_ID id)
	{
		// Remove id from Deployed and put id back to undeployed
		m_DeployedEntityList.erase(id);
		m_UndeployedEntityList.emplace(id);
		ECSManager::Instance().GetRegistry().release(m_AllEntityList[id]->m_Entity);
		// Readd Basic Components
		m_AllEntityList[id]->AddComponent<Properties>().m_Name = "AllocatedEntity";
		m_AllEntityList[id]->AddComponent<Transform>();
		m_AllEntityList[id]->AddComponent<Undeployed>();
	}

	/* !
	@function	AllocateEntitySize
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params		size_t size_ [Adds new amount of size_ into objects]

	@brief		Adds the additional amount of objects into objects

	*//*__________________________________________________________________________*/
	bool MemoryManager::AllocateEntitySize(size_t size)
	{
		// Reserve size for objects and registry in ECSManager
		m_AllEntityList.reserve(m_AllEntityList.size() + size);
		ECSManager::Instance().GetRegistry().reserve(ECSManager::Instance().GetRegistry().size() + size);

		// Resets and create objects based on size
		for (size_t i{}; i < size; ++i)
		{
			Entity obj{ std::make_shared<Ent>() };
			obj->m_Entity = ECSManager::Instance().GetRegistry().create();
			m_AllEntityList.emplace(static_cast<Entity_ID>(obj->m_Entity), obj);
			m_UndeployedEntityList.emplace(static_cast<Entity_ID>(obj->m_Entity));
			obj->AddComponent<Properties>().m_Name = "AllocatedEntity";
			obj->AddComponent<Transform>();
			obj->AddComponent<Undeployed>();
		}

		// Successful Allocation
		return true;
	}

	/* !
	@function	DeleteEntities
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params

	@brief		Deletes all entities in m_AllEntityList

	*//*__________________________________________________________________________*/
	bool MemoryManager::DeleteEntities()
	{
		for (auto& object : m_AllEntityList)
		{
			// Remove from m_EntityList
			object.second->AbandonChildren();
			// Release all components and entity itself
			ECSManager::Instance().GetRegistry().destroy(object.second->m_Entity);
			// Free unique ptr from the object
			object.second.reset();
		}

		m_AllEntityList.clear();
		m_DeployedEntityList.clear();
		m_UndeployedEntityList.clear();

		// Successful deletion
		return true;
	}

	/* !
	@function	MemoryManager::ResetToConfig
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params

	@brief		Deletes all undeployed objects, leaving only
				x amount of objects/components (Deployed or Undeployed) available
				based on the config_obj & config_comp

				This means it is possible that there are more deployed objects
				or components than the config size

	*//*__________________________________________________________________________*/
	void MemoryManager::ResetToConfig()
	{
		// Auto clear all the Undeployed Entities if m_ConfigSize exceeds deployed size
		if (m_ConfigSize <= m_DeployedEntityList.size())
		{
			for (Entity_ID id : m_UndeployedEntityList)
			{
				ECSManager::Instance().GetRegistry().destroy(m_AllEntityList[id]->m_Entity);
				// Free unique ptr from the object
				m_AllEntityList[id].reset();
				m_AllEntityList.erase(id);
			}
			m_UndeployedEntityList.clear();
		}

		size_t remainingSize{ m_ConfigSize - m_DeployedEntityList.size() };

		for (size_t i{}; i < remainingSize; ++i)
		{
			Entity_ID id{ *m_UndeployedEntityList.rbegin()};
			ECSManager::Instance().GetRegistry().destroy(m_AllEntityList[id]->m_Entity);
			// Free unique ptr from the object
			m_AllEntityList[id].reset();
			m_AllEntityList.erase(id);
			// Remove from undeployed
			m_UndeployedEntityList.erase(id);
		}
	}

	void MemoryManager::SetConfigSize(size_t configSize)
	{
		m_ConfigSize = configSize;
	}

	size_t MemoryManager::GetConfigSize() const
	{
		return m_ConfigSize;
	}
}