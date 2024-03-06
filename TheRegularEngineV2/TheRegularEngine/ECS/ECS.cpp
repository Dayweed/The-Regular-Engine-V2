#include "pch.h"
#include "ECS.h"
#include "Core/MemoryManager.h"
#include "Core/Parent.h"
#include "SystemManager.h"
#include "TREIncludes.h"

#define TO DELETE
#include "Core/GameLoop.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"


namespace TRE
{
	ECSManager& ECSManager::Instance()
	{
		static ECSManager instance;
		return instance;
	}

	entt::registry& ECSManager::GetRegistry()
	{
		return m_Registry;
	}

	void ECSManager::DeleteRemovalEntities()
	{
		std::vector<Entity> RemovalEntities{ GetEntities<Removal>(true) };
		bool haveEntities{ !RemovalEntities.empty() };
		for (Entity& object : RemovalEntities)
		{
			// Remove from m_EntityList
			m_EntityOrder.erase(std::find(m_EntityOrder.begin(), m_EntityOrder.end(), object->GetComponent<Properties>().m_GUID));
			m_EntityList.erase(m_EntityList.find(object->GetComponent<Properties>().m_GUID));
			m_EnttIDList.erase(m_EnttIDList.find(static_cast<ENTTID>(object->m_Entity)));
			MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(object->m_Entity));
		}
		if (haveEntities) UpdateEntityOrder();
	}

	void ECSManager::DestroyAll()
	{
		// Do proper abadoning children and deletion if debug
		//for (auto& pair : m_EntityList)
		//{
		//	ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChildren(pair.second);
		//	MarkForDeletion(pair.second);
		//}
		//DeleteRemovalEntities();
		
		// Clear everything
		m_EntityOrder.clear();
		m_EntityList.clear();
		m_EnttIDList.clear();
		MemoryManager::Instance().DeleteEntities();
	}

	Entity ECSManager::CreateEntity(std::string name)
	{
		Entity obj{ MemoryManager::Instance().GetUndeployedEntity() };
		m_EntityOrder.emplace_back(obj->GetComponent<Properties>().m_GUID);
		m_EntityList.emplace(obj->GetComponent<Properties>().m_GUID, obj);
		m_EnttIDList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);
		obj->GetComponent<Properties>().m_Name = name;
		return obj;
	}

	void ECSManager::DestroyEntity(Entity& object)
	{
		// Remove from m_EntityList
		m_EntityOrder.erase(std::find(m_EntityOrder.begin(), m_EntityOrder.end(), object->GetComponent<Properties>().m_GUID));
		m_EntityList.erase(m_EntityList.find(object->GetComponent<Properties>().m_GUID));
		m_EnttIDList.erase(m_EnttIDList.find(static_cast<ENTTID>(object->m_Entity)));
		MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(object->m_Entity));
		UpdateEntityOrder();
	}

	void ECSManager::MarkForDeletion(Entity& object)
	{
		if (!object->HasComponent<Removal>())
		{
			object->AddComponent<Removal>();
		}
		return;
	}

	Entity ECSManager::CloneEntity(Entity& object, std::string name)
	{
		Entity obj{ MemoryManager::Instance().GetUndeployedEntity() };
		// Remove all components in one entity
		for (auto&& elem : ECSManager::Instance().GetRegistry().storage()) {
			elem.second.remove(obj->m_Entity);
		}

		// Clone each component of the object into the clone
		for (auto [id, source_storage] : m_Registry.storage())
		{
			auto destination_storage = ECSManager::Instance().GetRegistry().storage(id);
			if (destination_storage != nullptr && source_storage.contains(object->m_Entity))
			{
				if (!destination_storage->contains(obj->m_Entity))
				{
					destination_storage->emplace(obj->m_Entity, source_storage.get(object->m_Entity));
				}
				// Overwrite m_Entity if m_Entity already contains the component
				else
				{
					destination_storage->erase(obj->m_Entity);
					destination_storage->emplace(obj->m_Entity, source_storage.get(object->m_Entity));
				}
			}
		}

		// Change Name
		obj->GetComponent<Properties>().m_Name = name;
		obj->GetComponent<Properties>().m_GUID = MemoryManager::Instance().GenerateGUIDStr();
		m_EntityOrder.emplace_back(obj->GetComponent<Properties>().m_GUID);
		m_EntityList.emplace(obj->GetComponent<Properties>().m_GUID, obj);
		m_EnttIDList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);

		// Construct if it is not displaying prefab!
		if (!GameLoop::Instance().GetDisplayingPrefab())
		{
			ConstructPhysicPrefab(obj);
		}

		// Return clone
		return obj;
	}

	// TO CHANGE
	void ECSManager::ConstructPhysicPrefab(Entity parent)
	{
		// Construct RigidBody, Sphere, Box or Capsule (TO CHANGE)
		parent->HasComponent<Rigidbody>() && ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructRigidbody(parent);
		if (parent->HasComponent<SphereCollider>())
		{
			SphereCollider& sc{ parent->GetComponent<SphereCollider>() };
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructSphereCollider(parent, sc.m_Radius, sc.m_Offset);
		}
		if (parent->HasComponent<BoxCollider>())
		{
			BoxCollider& bc{ parent->GetComponent<BoxCollider>() };
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructBoxCollider(parent, bc.m_HalfExtents, bc.m_Offset);
		}
		if (parent->HasComponent<CapsuleCollider>())
		{
			CapsuleCollider& cpc{ parent->GetComponent<CapsuleCollider>() };
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructCapsuleCollider(parent, cpc.m_Radius, cpc.m_HalfHeight);
		}
		if (parent->HasComponent<CylinderCollider>())
		{
			CylinderCollider& cyc{ parent->GetComponent<CylinderCollider>() };
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructCylinderCollider(parent, cyc.m_Radius, cyc.m_Height);
		}
	}

	void ECSManager::UpdateEntityChildProperties(std::string parentGUID)
	{
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->UpdateChildActive(FindEntity(parentGUID));
	}

	void ECSManager::UpdateEntityOrder()
	{
		for (int i{}; i < m_EntityOrder.size(); ++i)
		{
			m_EntityList[m_EntityOrder[i]]->GetComponent<Properties>().m_Index = i;
		}
	}

	void ECSManager::SortEntityOrder()
	{
		// Sort
		std::sort(m_EntityOrder.begin(), m_EntityOrder.end(),
			[&](std::string& l_GUID, std::string& r_GUID) {
				return m_EntityList[l_GUID]->GetComponent<Properties>().m_Index < m_EntityList[r_GUID]->GetComponent<Properties>().m_Index;
			});

		int currentIndex = 0;
		// Sort their children also based on order
		for (int i{}; i < m_EntityOrder.size(); ++i)
		{
			// Only for main parent
			if (m_EntityList[m_EntityOrder[i]]->GetComponent<Parenting>().m_Parent == "")
			{
				std::vector<std::string>& children{ m_EntityList[m_EntityOrder[i]]->GetComponent<Parenting>().m_Children };
				std::sort(children.begin(), children.end(),
					[&](std::string& l_GUID, std::string& r_GUID) {
						return m_EntityList[l_GUID]->GetComponent<Properties>().m_Index < m_EntityList[r_GUID]->GetComponent<Properties>().m_Index;
					});
				// Change it be index based on the parent
				m_EntityList[m_EntityOrder[i]]->GetComponent<Properties>().m_Index = currentIndex++;
				UpdateChildrenOrder(m_EntityList[m_EntityOrder[i]], currentIndex);
			}
		}

		// Sort
		std::sort(m_EntityOrder.begin(), m_EntityOrder.end(),
			[&](std::string& l_GUID, std::string& r_GUID) {
				return m_EntityList[l_GUID]->GetComponent<Properties>().m_Index < m_EntityList[r_GUID]->GetComponent<Properties>().m_Index;
			});

		// Update their order
		UpdateEntityOrder();
	}

	std::vector<Entity> ECSManager::GetAllEntities(bool IncludeNonActive)
	{
		std::vector<Entity> objects{};
		objects.reserve(m_EntityOrder.size());

		// Get all Entity owning the entities
		for (std::string& id : m_EntityOrder)
		{
			if (IncludeNonActive || m_Registry.get<Properties>(m_EntityList[id]->m_Entity).m_Active)
			{
				objects.emplace_back(m_EntityList[id]);
			}
		}

		return objects;
	}

	std::string ECSManager::SaveEntities(std::string filePath)
	{
		// Set up document
		ECSOutputArchive arc(filePath);

		entt::snapshot snapshot{ GetRegistry() };
		// REMEMBER TO UPDATE Prefab.cpp TOO!!!
		
		// Serialize all entities and components
		snapshot.entities(arc)
			.component<Prefabing>(arc)
			.component<Parenting>(arc)
			.component<Properties>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<Rigidbody>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<CapsuleCollider>(arc)
			.component<DirectionalLight>(arc)
			.component<AudioListener>(arc)
			.component<Audio>(arc)
			.component<ScriptComponent>(arc)
			.component<UIComponent>(arc)
			.component<AnimationComponent>(arc)
			.component<Particle2DComponent>(arc)
			.component<DirectPathfinding>(arc)
			.component<TextComponent>(arc)
			.component<CylinderCollider>(arc)
			.component<SlideshowComponent>(arc)
			.component<Sprite3DComponent>(arc)
			.component<Particle3DComponent>(arc)
			;

		arc.Close();

		return arc.GetFilePath();
	}

	void ECSManager::LoadEntities(std::string filePath)
	{
		MemoryManager::Instance().DeleteEntities();

		entt::registry copy;
		ECSInputArchive arc(filePath);

		// REMEMBER TO UPDATE Prefab.cpp TOO!!!
		entt::basic_snapshot_loader loader(copy);
		loader.entities(arc)
			.component<Prefabing>(arc)
			.component<Parenting>(arc)
			.component<Properties>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<Rigidbody>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<CapsuleCollider>(arc)
			.component<DirectionalLight>(arc)
			.component<AudioListener>(arc)
			.component<Audio>(arc)
			.component<ScriptComponent>(arc)
			.component<UIComponent>(arc)
			.component<AnimationComponent>(arc)
			.component<Particle2DComponent>(arc)
			.component<DirectPathfinding>(arc)
			.component<TextComponent>(arc)
			.component<CylinderCollider>(arc)
			.component<SlideshowComponent>(arc)
			.component<Sprite3DComponent>(arc)
			.component<Particle3DComponent>(arc)
			;

		MemoryManager::Instance().UpdateECSManager(copy);
	}

	Entity ECSManager::FindEntity(std::string id)
	{
		if (m_EntityList.find(id) != m_EntityList.end())
		{
			return m_EntityList[id];
		}

		return nullptr;
	}

	std::string ECSManager::FindEntityID(Entity ent)
	{
		return ent->GetComponent<Properties>().m_GUID;
	}

	Entity ECSManager::FindEntityName(std::string name)
	{
		for (auto ent : m_EntityList)
		{
			if (ent.first == name) return ent.second;
		}

		return nullptr;
	}

	bool ECSManager::IsRemovableComponent(std::string compName)
	{
		if (m_CompRemovable.find(compName) != m_CompRemovable.end())
		{
			return m_CompRemovable[compName];
		}
		// Return false if doesnt exist
		return false;
	}

	void ECSManager::AddCompFromName(Entity ent, std::string compName)
	{
		m_AddCompFunctions[compName](ent);
	}

	void ECSManager::RemCompFromName(Entity ent, std::string compName)
	{
		m_RemCompFunctions[compName](ent);
	}

	void ECSManager::SaveRegistry(entt::registry& dstRegistry)
	{
		dstRegistry.clear();

		// Ensure it knows these components exists
		(void)dstRegistry.view<
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
			Audio,
			AudioListener,
			DirectionalLight,
			ScriptComponent,
			UIComponent,
			AnimationComponent,
			Particle2DComponent,
			DirectPathfinding,
			TextComponent,
			SlideshowComponent,
			Sprite3DComponent,
			Particle3DComponent
		>();

		m_Registry.each([&](entt::entity srcEntity)
			{
				entt::entity dstEntity = dstRegistry.create();

				for (auto [id, source_storage] : m_Registry.storage())
				{
					auto destination_storage = dstRegistry.storage(id);
					if (destination_storage != nullptr && source_storage.contains(srcEntity))
					{
						if (!destination_storage->contains(dstEntity))
						{
							destination_storage->emplace(dstEntity, source_storage.get(srcEntity));
						}
						// Overwrite m_Entity if m_Entity already contains the component
						else
						{
							destination_storage->erase(dstEntity);
							destination_storage->emplace(dstEntity, source_storage.get(srcEntity));
						}
					}
				}
			});
	}

	void ECSManager::CopyRegistry(entt::registry& srcRegistry)
	{
		MemoryManager::Instance().DeleteEntities();

		// Copy 
		MemoryManager::Instance().UpdateECSManager(srcRegistry);
	}

	void ECSManager::AddToRegistry(entt::registry& srcRegistry)
	{
		// Keep adding into it 
		MemoryManager::Instance().UpdateECSManager(srcRegistry);
	}

	bool ECSManager::IsValidEntity(Entity ent)
	{
		// Checks if it is in the scene
		return ent->HasComponent<Properties>() && m_EntityList.find(ent->GetGUID()) != m_EntityList.end() && std::find(m_EntityOrder.begin(), m_EntityOrder.end(), ent->GetGUID()) != m_EntityOrder.end();
	}

	void ECSManager::UpdateChildrenOrder(Entity child, int& order)
	{
		std::vector<std::string>& children{ child->GetComponent<Parenting>().m_Children };
		for (int c{}; c < children.size(); ++c)
		{
			if (ECSManager::Instance().IsValidEntity(m_EntityList[children[c]]))
			{
				m_EntityList[children[c]]->GetComponent<Properties>().m_Index = order++;
				UpdateChildrenOrder(m_EntityList[children[c]], order);
			}
			else
			{
				//TRE_WARN("[ECSManager::UpdateChildrenOrder] Entity (" + child->GetComponent<Properties>().m_Name ") have an invalid child at " + c + "!");
			}
		}
	}

	std::vector<std::pair<std::string, property::base*>> ECSManager::GetAllInspectableComponents(Entity object)
	{
		std::vector<std::pair<std::string, property::base*>> components;
		for (auto&& curr : m_Registry.storage())
		{
			if (auto& storage = curr.second; storage.contains(object->m_Entity))
			{
				if (m_PropertyBased.find(curr.first) != m_PropertyBased.end())
				{
					components.push_back({ m_PropertyBased[curr.first], static_cast<property::base*>(storage.get(object->m_Entity)) });
				}
			}
		}
		return components;
	}

	std::vector<std::string> ECSManager::GetAllNonAddedComponents(Entity object)
	{
		// Get all existing comp
		std::vector<std::pair<std::string, property::base*>> exisingComp{ GetAllInspectableComponents(object) };
		std::vector<std::string> existingCompName;
		for (auto& c : exisingComp)
		{
			existingCompName.emplace_back(c.first);
		}

		// Add all non-existing comp into vector
		std::vector<std::string> compName;
		for (auto& compPair : ComponentManager::Instance().GetImguiAddComp())
		{
			if (std::find(existingCompName.begin(), existingCompName.end(), compPair.second) == existingCompName.end())
			{
				compName.emplace_back(compPair.second);
			}
		}

		return compName;
	}

	Entity Ent::GetThis()
	{
		return shared_from_this();
	}

	ENTTID Ent::GetENTTID()
	{
		return static_cast<ENTTID>(m_Entity);
	}

	std::string Ent::GetName()
	{
		return GetComponent<Properties>().m_Name;
	}

	std::string Ent::GetGUID()
	{
		return GetComponent<Properties>().m_GUID;
	}

	ECSOutputArchive::ECSOutputArchive(std::string fileName) : m_FileName(fileName)
	{
		m_Root = nlohmann::json::array();
	}

	void ECSOutputArchive::operator()(entt::entity ent)
	{
		m_Current.push_back(static_cast<uint32_t>(ent));
	}

	void ECSOutputArchive::operator()(std::underlying_type_t<entt::entity> u)
	{
		// First element of each array keeps the amount of elements. 
		if (m_Current.empty()) {
			m_Current = nlohmann::json::array();
			m_Current.push_back(u);
		}
		else
		{
			m_Root.push_back(m_Current);
			m_Current = nlohmann::json::array();
			m_Current.push_back(u);
		}
	}

	void ECSOutputArchive::Close()
	{
		if (!m_Current.empty()) {
			m_Root.push_back(m_Current);
		}

		std::filesystem::path path{ m_FileName };
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << m_Root.dump(3);
		file.close();
	}

	std::string ECSOutputArchive::AsString()
	{
		return m_Root.dump();
	}

	std::string ECSOutputArchive::GetFilePath()
	{
		return m_FileName;
	}

	ECSInputArchive::ECSInputArchive(std::string fileName) : m_FileName(fileName)
	{
		std::ifstream file(m_FileName);
		m_Root = nlohmann::json::parse(file);
	}

	void ECSInputArchive::operator()(entt::entity& ent)
	{
		uint32_t entID = m_Current[m_CurrentIdx].get<uint32_t>();
		ent = entt::entity(entID);
		m_CurrentIdx++;
	}

	void ECSInputArchive::operator()(std::underlying_type_t<entt::entity>& u)
	{
		m_RootIdx++;
		if (m_RootIdx >= m_Root.size())
		{
			std::string funcName{ __FUNCTION__ };
			std::string error{ "[" + funcName + "] InputArchive have m_RootIdx " + std::to_string(m_RootIdx) + " < " + std::to_string(m_Root.size())
				+ "! This means there is a new component! Assuming it doesn't have it..." };
			TRE_CORE_WARN(error);
			return;
		}
		m_Current = m_Root[m_RootIdx];
		m_CurrentIdx = 0;

		int size = m_Current[0].get<int>();
		m_CurrentIdx++;
		// Pass amount to entt
		u = static_cast<std::underlying_type_t<entt::entity>>(size); 
	}
}