#pragma once
/*!
	@file		EntityCopy.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		20/10/2023
	@brief		Handles copying and pasting entities
				It can also handle copy Entities from ECSManager Registry
				to other registries!

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

#include "ECS.h"

namespace TRE
{
	class EntityCopier
	{
	public:
		static EntityCopier& Instance()
		{
			static EntityCopier instance;
			return instance;
		}

		void Shutdown();

		void CopyEntities(Entity srcObj);

		void PasteEntities();

		void SaveEntityInRegistry(Entity object, entt::registry& dstReg, std::string parentGUID = "", entt::entity parentEnt = {});		// Entity must be from the ECSManager::Instance().GetRegistry()!

		void GenerateNewGUID(entt::entity ent, entt::registry& reg);

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		EntityCopier();
		EntityCopier(EntityCopier const&) = delete;
		void operator=(EntityCopier const&) = delete;
		void* operator new(size_t) = delete;

		entt::registry m_CopierRegistry;
	};
}