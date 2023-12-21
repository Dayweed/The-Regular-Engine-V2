#pragma once
/*!
	@file		MemoryManager.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		2/09/2023
	@brief		This file handles all allocations for entt::entities

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                 includes
----------------------------------------------------------------------------- */
#include "pch.h"
#include "ECS.h"
#include <map>

/*                                                                 defines
----------------------------------------------------------------------------- */
#define MEM_MGR_DEFAULT_NAME "AllocatedEntity"

namespace TRE
{
	// Memory Manager [The ONLY ONE to handle new and delete]
	//==================================================
	class MemoryManager // Handles Memory Allocation and should be run once at the start and end of main!
	{
	public:
		/* !
		@function		Instance
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Creates a static instance of the MemoryManager

		Example:
		MemoryManager::Instance().DeleteEntities();
		*//*__________________________________________________________________________*/
		static MemoryManager& Instance();

		// ECS Manager
		/* !
		@function		GetUndeployedEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns an Allocated Entity that is not deployed
		*//*__________________________________________________________________________*/
		Entity& GetUndeployedEntity();

		/* !
		@function		ReleaseDeployedEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			id	Entity_ID of the object to release

		@brief			Release a deployed Entity based on the id
		*//*__________________________________________________________________________*/
		void ReleaseDeployedEntity(ENTTID id);

		/* !
		@function		AllocateEntitySize
		@author			Isaiah Lim Ji Rong (lim.i@digipen.edu)

		@params			size_	Adds new amount of size_ into objects

		@brief			Adds the additional amount of objects into objects
		*//*__________________________________________________________________________*/
		//bool AllocateEntitySize(size_t size_);

		/* !
		@function		DeleteEntities
		@author			Isaiah Lim Ji Rong  (lim.i@digipen.edu)

		@brief			Deletes all entities in m_AllEntityList
		*//*__________________________________________________________________________*/
		bool DeleteEntities();

		/* !
		@function		ResetToConfig
		@author			Isaiah Lim Ji Rong (lim.i@digipen.edu)

		@brief			Deletes x amount of undeployed entities
						based on the m_ConfigSize
						Leaves deployed entities untouched

						This means it is possible that there are more deployed entities
						than the config size
		*//*__________________________________________________________________________*/
		//void ResetToConfig();

		/* !
		@function		ClearUndeployed
		@author			Isaiah Lim Ji Rong (lim.i@digipen.edu)

		@brief			Deletes all of the undeployed entities
						Leaves deployed entities untouched

						This means it is possible that there are more deployed entities
						than the config size
		*//*__________________________________________________________________________*/
		//void ClearUndeployed();

		/* !
		@function		UpdateECSManager
		@author			Isaiah Lim Ji Rong (lim.i@digipen.edu)

		@params			reg	Source Registry to copy entities from

		@brief			Copies entt::entities from the Source Registry to the
						ECSManager m_Registry
		*//*__________________________________________________________________________*/
		void UpdateECSManager(entt::registry& reg);

		/* !
		@function		GenerateGUIDStr
		@author			Isaiah Lim Ji Rong (lim.i@digipen.edu)

		@brief			Generates the GUID string for entities
		*//*__________________________________________________________________________*/
		std::string GenerateGUIDStr();

		// Set and Get
		//void SetConfigSize(size_t config_obj_);
		//size_t GetConfigSize() const;

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		MemoryManager() {};
		MemoryManager(MemoryManager const&) = delete;
		void operator=(MemoryManager const&) = delete;
		void* operator new(size_t) = delete;

		// Size set in config
		//size_t								m_ConfigSize{ 100 };

		std::unordered_map<ENTTID, Entity>	m_AllEntityList;
		//std::set<ENTTID>					m_DeployedEntityList;
		//std::set<ENTTID>					m_UndeployedEntityList;
	};
}