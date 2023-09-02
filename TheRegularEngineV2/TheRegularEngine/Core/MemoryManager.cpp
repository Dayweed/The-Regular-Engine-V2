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
#include "ECS.h"

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

	/* !
	@function	AllocateObjectSize
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params		size_t size_ [Adds new amount of size_ into objects]

	@brief		Adds the additional amount of objects into objects

	*//*__________________________________________________________________________*/
	bool MemoryManager::AllocateEntitySize(size_t size_)
	{
		// Resets and create objects based on size_
		for (size_t i{}; i < size_; ++i)
		{
			Entity obj{ std::make_shared<Ent>() };
			obj->m_Entity = ECSManager::Instance().GetRegistry().create();
			ECSManager::Instance().m_EntityList.emplace(static_cast<uint32_t>(obj->m_Entity), obj);
			obj->AddComponent<Properties>().m_Name = "AllocatedEntity";
			obj->AddComponent<Transform>();
		}

		// Successful Allocation
		return true;
	}

	/* !
	@function	DeleteObjects
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params

	@brief		Deletes all objects in objects

	*//*__________________________________________________________________________*/
	bool MemoryManager::DeleteEntities()
	{
		for (auto& object : ECSManager::Instance().m_EntityList)
		{
			// Remove from m_EntityList
			object.second->AbandonChildren();
			// Release all components and entity itself
			ECSManager::Instance().registry.destroy(object.second->m_Entity);
			// Free unique ptr from the object
			object.second.reset();
		}

		/*for (auto& object : ECSManager::Instance().m_EntityList)
		{
			delete object;
			object = NULL;
		}
		_ecs_manager->objects.clear();
		_ecs_manager->undeployed_objects.clear();
		_ecs_manager->deployed_objects.clear();*/

		// Successful deletion
		return true;
	}

	/* !
	@function	MemoryManager::ResetToConfig
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params

	@brief		Deletes all undeployed objects and components, leaving only
				x amount of objects/components (Deployed or Undeployed) available
				based on the config_obj & config_comp

				This means it is possible that there are more deployed objects
				or components than the config size

	*//*__________________________________________________________________________*/
	void MemoryManager::ResetToConfig()
	{
		//for (int i{ static_cast<int>(_ecs_manager->objects.size()) - 1 }; i >= 0; --i)
		//{
		//	// Delete if objects is undeployed and there are more existing objects than the config_obj
		//	if (!_ecs_manager->objects[i]->GetDeployed() && _ecs_manager->objects.size() > config_obj)
		//	{
		//		// Remove from undeployed_objects
		//		if (std::find(_ecs_manager->undeployed_objects.begin(), _ecs_manager->undeployed_objects.end(), _ecs_manager->objects[i]) != _ecs_manager->undeployed_objects.end())
		//		{
		//			_ecs_manager->undeployed_objects.erase(std::find(_ecs_manager->undeployed_objects.begin(), _ecs_manager->undeployed_objects.end(), _ecs_manager->objects[i]));
		//		}
		//		// Remove from deployed_objects
		//		if (std::find(_ecs_manager->deployed_objects.begin(), _ecs_manager->deployed_objects.end(), _ecs_manager->objects[i]) != _ecs_manager->deployed_objects.end())
		//		{
		//			_ecs_manager->deployed_objects.erase(std::find(_ecs_manager->deployed_objects.begin(), _ecs_manager->deployed_objects.end(), _ecs_manager->objects[i]));
		//			std::cout << "MemoryManager::ResetToConfig(): (Object) This should NEVER happen\n";
		//		}
		//		delete _ecs_manager->objects[i];
		//		_ecs_manager->objects.erase(_ecs_manager->objects.begin() + i);
		//	}
		//}
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