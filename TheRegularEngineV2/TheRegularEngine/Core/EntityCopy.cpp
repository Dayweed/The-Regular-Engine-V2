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
			Rigidbody,
			SphereCollider,
			BoxCollider,
			CapsuleCollider,
			CylinderCollider,
			DirectionalLight,
			ScriptComponent,
			Audio,
			AudioListener,
			ParticleComponent,
			DirectPathfinding,
			TextComponent,
			SlideshowComponent
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

	Entity EntityCopier::PasteEntities()
	{
		ECSSystemManager::Instance().BeforeReset();
		// Ensure all the copied registry has new guid again jic for duplication
		std::vector<std::string> pastedGUIDs;
		m_CopierRegistry.each([&](entt::entity srcEntity) {
			pastedGUIDs.emplace_back(GenerateNewGUID(srcEntity, m_CopierRegistry));
		});
		ECSManager::Instance().AddToRegistry(m_CopierRegistry);
		// Add these entities to their parents
		ParentingSystem* parentSystem{ ECSSystemManager::Instance().GetSystem<ParentingSystem>() };
		for (size_t i{}; i < pastedGUIDs.size(); ++i)
		{
			std::string pastedGUID{ pastedGUIDs[i] };
			Parenting& parentComp{ ECSManager::Instance().FindEntity(pastedGUID)->GetComponent<Parenting>() };
			// Assign to parent if it exists
			if (Entity parent{ ECSManager::Instance().FindEntity(parentComp.m_Parent) }; parent)
			{
				parentSystem->AddChild(parent, ECSManager::Instance().FindEntity(pastedGUID));
			}
			else
			{
				parentComp.m_Parent = "";
			}
		}
		ECSSystemManager::Instance().AfterReset();
		return ECSManager::Instance().FindEntity(pastedGUIDs.front());
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

	std::string EntityCopier::GenerateNewGUID(entt::entity ent, entt::registry& reg)
	{
		// Force serialize new guid for the ent
		std::string prevGUID = reg.get<Properties>(ent).m_GUID;
		std::string entGUID = MemoryManager::Instance().GenerateGUIDStr();
		reg.get<Properties>(ent).m_GUID = entGUID;

		reg.each([&](entt::entity srcEntity) {
			// Replace it's childrens parent
			if (reg.get<Parenting>(srcEntity).m_Parent == prevGUID)
			{
				reg.get<Parenting>(srcEntity).m_Parent = entGUID;
			}
			// Replace it's parent's children
			else
			{
				std::vector<std::string>& children{ reg.get<Parenting>(srcEntity).m_Children };
				auto it = std::find(children.begin(), children.end(), prevGUID);
				if (it != children.end())
				{
					children.erase(it);
					children.emplace_back(entGUID);
				}
			}
		});

		return entGUID;
	}
}