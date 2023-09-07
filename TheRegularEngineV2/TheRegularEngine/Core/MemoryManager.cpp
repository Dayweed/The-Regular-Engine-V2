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
#include <combaseapi.h>
#include <atlconv.h>

namespace TRE
{
	MemoryManager& MemoryManager::Instance()
	{
		static MemoryManager instance{};
		return instance;
	}

	Entity& MemoryManager::GetUndeployedEntity()
	{
		// Allocate additional space if there is no undeployed
		if (m_UndeployedEntityList.empty())
		{
			if (m_AllEntityList.empty())
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_ERROR("[" + funcName + "] m_AllEntityList is no empty! (Remember to allocate size for MemoryManager at RegisterECS)");
				assert(!m_AllEntityList.empty());
			}

			AllocateEntitySize(m_AllEntityList.size());
		}

		// Get an Entity_ID from Undeployed and assign it to Deployed, returns the Entity
		ENTTID id{ *m_UndeployedEntityList.begin() };
		m_UndeployedEntityList.erase(id);
		m_DeployedEntityList.emplace(id);
		m_AllEntityList[id]->RemoveComponent<Undeployed>();

		m_AllEntityList[id]->GetComponent<Properties>().m_GUID = GenerateGUIDStr();

		return m_AllEntityList[id];
	}

	void MemoryManager::ReleaseDeployedEntity(ENTTID id)
	{
		// Remove id from Deployed and put id back to undeployed
		m_DeployedEntityList.erase(id);
		m_UndeployedEntityList.emplace(id);
		// Remove all components in one entity
		for (auto&& elem : ECSManager::Instance().GetRegistry().storage()) {
			elem.second.remove(m_AllEntityList[id]->m_Entity);
		}
		// Readd Basic Components
		m_AllEntityList[id]->AddComponent<Properties>().m_Name = MEM_MGR_DEFAULT_NAME;
		m_AllEntityList[id]->AddComponent<Parenting>();
		m_AllEntityList[id]->AddComponent<Transform>();
		m_AllEntityList[id]->AddComponent<Undeployed>();
	}

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

			m_AllEntityList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);
			m_UndeployedEntityList.emplace(static_cast<ENTTID>(obj->m_Entity));
			if (!obj->HasComponent<Properties>())
			{
				obj->AddComponent<Properties>().m_Name = MEM_MGR_DEFAULT_NAME;
			}
			else
			{
				obj->GetComponent<Properties>().m_Name = MEM_MGR_DEFAULT_NAME;
			}
			if (!obj->HasComponent<Parenting>())
			{
				obj->AddComponent<Parenting>();
			}
			if (!obj->HasComponent<Transform>())
			{
				obj->AddComponent<Transform>();
			}
			if (!obj->HasComponent<Undeployed>())
			{
				obj->AddComponent<Undeployed>();
			}
		}

		// Successful Allocation
		return true;
	}

	bool MemoryManager::DeleteEntities()
	{
		for (auto& object : m_AllEntityList)
		{
			// Remove from m_EntityList
			object.second->AbandonChildren();
			// Release all components and entity itself
			if (ECSManager::Instance().GetRegistry().valid(object.second->m_Entity))
				ECSManager::Instance().GetRegistry().destroy(object.second->m_Entity);
			// Free unique ptr from the object
			object.second.reset();
		}

		m_AllEntityList.clear();
		m_DeployedEntityList.clear();
		m_UndeployedEntityList.clear();

		ECSManager::Instance().m_EntityList.clear();
		ECSManager::Instance().GetRegistry().clear();

		// Successful deletion
		return true;
	}

	void MemoryManager::ResetToConfig()
	{
		// Auto clear all the Undeployed Entities if m_ConfigSize exceeds deployed size
		if (m_ConfigSize <= m_DeployedEntityList.size())
		{
			for (ENTTID id : m_UndeployedEntityList)
			{
				ECSManager::Instance().GetRegistry().destroy(m_AllEntityList[id]->m_Entity);
				// Free unique ptr from the object
				m_AllEntityList[id].reset();
				m_AllEntityList.erase(id);
			}
			m_UndeployedEntityList.clear();
		}
		else if (!m_UndeployedEntityList.empty())
		{
			size_t remainingSize{ m_ConfigSize - m_DeployedEntityList.size() };

			for (size_t i{}; i < remainingSize && !m_UndeployedEntityList.empty(); ++i)
			{
				ENTTID id{ *m_UndeployedEntityList.rbegin() };
				ECSManager::Instance().GetRegistry().destroy(m_AllEntityList[id]->m_Entity);
				// Free unique ptr from the object
				m_AllEntityList[id].reset();
				m_AllEntityList.erase(id);
				// Remove from undeployed
				m_UndeployedEntityList.erase(id);
			}
		}

		AllocateEntitySize(m_ConfigSize);
	}


	void MemoryManager::ClearUndeployed()
	{
		for (ENTTID id : m_UndeployedEntityList)
		{
			ECSManager::Instance().GetRegistry().destroy(m_AllEntityList[id]->m_Entity);
			// Free unique ptr from the object
			m_AllEntityList[id].reset();
			m_AllEntityList.erase(id);
		}
		m_UndeployedEntityList.clear();
	}

	void MemoryManager::UpdateECSManager(entt::registry& reg)
	{
		// Update ECS Manager based on current registry
		reg.each([&](entt::entity srcEntity) {

			Entity obj{ std::make_shared<Ent>() };
			obj->m_Entity = ECSManager::Instance().GetRegistry().create();

			m_AllEntityList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);
			m_DeployedEntityList.emplace(static_cast<ENTTID>(obj->m_Entity));
			
			for (auto [id, source_storage] : reg.storage())
			{
				auto destination_storage = ECSManager::Instance().GetRegistry().storage(id);
				if (destination_storage != nullptr && source_storage.contains(srcEntity))
				{
					if (!destination_storage->contains(obj->m_Entity))
					{
						destination_storage->emplace(obj->m_Entity, source_storage.get(srcEntity));
					}
					// Overwrite m_Entity if m_Entity already contains the component
					else
					{
						destination_storage->erase(obj->m_Entity);
						destination_storage->emplace(obj->m_Entity, source_storage.get(srcEntity));
					}
				}
			}

			ECSManager::Instance().m_EntityList.emplace(obj->GetComponent<Properties>().m_GUID, obj);
		});

		ResetToConfig();
	}

	std::string MemoryManager::GenerateGUIDStr()
	{
		GUID guid;
		HRESULT result{ CoCreateGuid(&guid) };
		LPOLESTR guidLPOLEStr;
		result = StringFromCLSID(guid, &guidLPOLEStr);
		USES_CONVERSION;
		return OLE2CA(guidLPOLEStr);
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