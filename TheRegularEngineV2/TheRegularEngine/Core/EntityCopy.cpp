#include "pch.h"
#include "EntityCopy.h"
#include "TREIncludes.h"

namespace TRE
{
	EntityCopier::EntityCopier()
	{
		(void)m_CopierRegistry.view<
			Prefabing,
			Parenting,
			Properties,
			Transform,
			MeshRenderer,
			Camera,
			SphereCollider,
			BoxCollider,
			Rigidbody,
			FEL,
			FAKEFEL,
			DirectionalLight,
			ScriptComponent
		>();
	};

	void EntityCopier::Shutdown()
	{
		m_CopierRegistry.clear();
	}

	void EntityCopier::CopyEntities(Entity srcObj)
	{
		if (srcObj == nullptr) return;
		m_CopierRegistry.clear();
		SaveEntityInRegistry(srcObj, m_CopierRegistry);
	}

	void EntityCopier::PasteEntities()
	{
		ECSManager::Instance().AddToRegistry(m_CopierRegistry);
	}

	void EntityCopier::SaveEntityInRegistry(Entity object, entt::registry& dstReg, std::string parentGUID, entt::entity parentEnt)
	{
		// Create prefab
		entt::entity ent = dstReg.create();

		// Clone each component of the object into the prefab
		for (auto [id, source_storage] : ECSManager::Instance().GetRegistry().storage())
		{
			auto destination_storage = dstReg.storage(id);
			if (destination_storage != nullptr && source_storage.contains(object->m_Entity))
			{
				if (!destination_storage->contains(ent))
				{
					destination_storage->emplace(ent, source_storage.get(object->m_Entity));
				}
				// Overwrite m_Entity if m_Entity already contains the component
				else
				{
					destination_storage->erase(ent);
					destination_storage->emplace(ent, source_storage.get(object->m_Entity));
				}
			}
		}

		// Force serialize new guid for the ent
		std::string prevGUID = dstReg.get<Properties>(ent).m_GUID;
		std::string entGUID = MemoryManager::Instance().GenerateGUIDStr();
		dstReg.get<Properties>(ent).m_GUID = entGUID;

		// Update it's parenting, if it have to update their parent guid
		if (parentGUID != "")
		{
			dstReg.get<Parenting>(ent).m_Parent = parentGUID;
			// Remove previous GUID string and add new id
			std::vector<std::string>& children{ dstReg.get<Parenting>(parentEnt).m_Children };
			auto it = std::find(children.begin(), children.end(), prevGUID);
			children.erase(it);
			children.emplace_back(entGUID);
		}

		// Clone it's children
		for (Entity child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(object))
		{
			SaveEntityInRegistry(child, dstReg, entGUID, ent);
		}
	}
}