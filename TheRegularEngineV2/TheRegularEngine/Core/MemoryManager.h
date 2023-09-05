#pragma once
/*!
	@file    MemoryManager.hpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    14/09/2022
	@brief   This file handles all new and delete related to ecs and checks for
			 mem leaks

	Copyright (C) 2022 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                 includes
----------------------------------------------------------------------------- */
#include "pch.h"
#include "ECS.h"
#include <map>

#define MEM_MGR_DEFAULT_NAME "AllocatedEntity"

namespace TRE
{
	// Memory Manager [The ONLY ONE to handle new and delete]
	//==================================================
	class MemoryManager // Handles Memory Allocation and should be run once at the start and end of main!
	{
	public:
		static MemoryManager& Instance();

		// ECS Manager
		Entity& GetUndeployedEntity();
		void ReleaseDeployedEntity(Entity_ID id);

		bool AllocateEntitySize(size_t size_);
		bool DeleteEntities();
		void ResetToConfig();
		void ClearUndeployed();

		void UpdateECSManager(entt::registry& reg);

		// Set and Get
		void SetConfigSize(size_t config_obj_);
		size_t GetConfigSize() const;

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		MemoryManager() {};
		MemoryManager(MemoryManager const&) = delete;
		void operator=(MemoryManager const&) = delete;
		void* operator new(size_t) = delete;

		// Size set in config
		size_t m_ConfigSize{ 100 };

		std::unordered_map<Entity_ID, Entity>	m_AllEntityList;
		std::set<Entity_ID>						m_DeployedEntityList;
		std::set<Entity_ID>						m_UndeployedEntityList;
	};
}