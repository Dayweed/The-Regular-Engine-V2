#include "pch.h"
#include "ECS.h"
#include "MemoryManager.h"
#include "Parent.h"
#include "SystemManager.h"
#include "TREIncludes.h"

#define TO DELETE
#include "Transform.h"
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
		for (Entity& object : GetEntities<Removal>())
		{
			// Remove from m_EntityList
			m_EntityList.erase(m_EntityList.find(object->GetComponent<Properties>().m_GUID));
			m_EnttIDList.erase(m_EnttIDList.find(static_cast<ENTTID>(object->m_Entity)));
			MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(object->m_Entity));
		}
	}

	void ECSManager::DestroyAll()
	{
		for (auto& pair : m_EntityList)
		{
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChildren(pair.second);
			MarkForDeletion(pair.second);
		}
		DeleteRemovalEntities();
	}

	Entity ECSManager::CreateEntity(std::string name)
	{
		Entity obj{ MemoryManager::Instance().GetUndeployedEntity() };
		m_EntityList.emplace(obj->GetComponent<Properties>().m_GUID, obj);
		m_EnttIDList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);
		obj->GetComponent<Properties>().m_Name = name;
		return obj;
	}

	void ECSManager::DestroyEntity(Entity& object)
	{
		// Remove from m_EntityList
		m_EntityList.erase(m_EntityList.find(object->GetComponent<Properties>().m_GUID));
		m_EnttIDList.erase(m_EnttIDList.find(static_cast<ENTTID>(object->m_Entity)));
		MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(object->m_Entity));
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
		/*for (auto&& curr : m_Registry.storage())
		{
			if (auto& storage = curr.second; storage.contains(object->m_Entity))
			{
				storage.emplace(obj->m_Entity, storage.get(object->m_Entity));
			}
		}*/
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
		m_EntityList.emplace(obj->GetComponent<Properties>().m_GUID, obj);
		m_EnttIDList.emplace(static_cast<ENTTID>(obj->m_Entity), obj);
		// Return clone
		return obj;
	}


	std::vector<Entity> ECSManager::GetAllEntities()
	{
		std::vector<Entity> objects{};
		objects.reserve(m_EntityList.size());

		// Get all Entity owning the entities
		for (auto& obj : m_EntityList)
		{
			objects.emplace_back(obj.second);
		}

		return objects;
	}

	std::string ECSManager::SaveEntities(std::string filePath)
	{
		// Set up document
		ECSOutputArchive arc(filePath);

		// Destroys all undeployed entities
		MemoryManager::Instance().ClearUndeployed();

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
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<Rigidbody>(arc)
			.component<DirectionalLight>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
			.component<AudioListener>(arc)
			.component<Audio>(arc)
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
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<Rigidbody>(arc)
			.component<DirectionalLight>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
			.component<AudioListener>(arc)
			.component<Audio>(arc)
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
		(void)dstRegistry.view<Prefabing, Parenting, Properties, Transform, MeshRenderer, Camera, SphereCollider, BoxCollider, Rigidbody, Audio, AudioListener, DirectionalLight, FEL, FAKEFEL>();

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
		MemoryManager::Instance().UpdateECSManager(srcRegistry, false);
	}

	void ECSManager::AddToRegistry(entt::registry& srcRegistry)
	{
		// Keep adding into it 
		MemoryManager::Instance().UpdateECSManager(srcRegistry, false);
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
		if (ECSManager::Instance().GetRegistry().valid(ent))
		{
			m_Current.push_back(static_cast<uint32_t>(ent));
		}
	}

	void ECSOutputArchive::operator()(std::underlying_type_t<entt::entity> u)
	{
		// First element of each array keeps the amount of elements. 
		if (m_Current.empty()) {
			m_Current = nlohmann::json::array();
			m_Current.push_back(ECSManager::Instance().GetAllEntities().size());
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
			std::string error{ "[" + funcName + "] InputArchive have m_RootIdx " + std::to_string(m_RootIdx) + " < " + std::to_string(m_Root.size()) };
			TRE_CORE_ERROR(error);
			assert(m_RootIdx < m_Root.size());
			return;
		}
		m_Current = m_Root[m_RootIdx];
		m_CurrentIdx = 0;

		int size = m_Current[0].get<int>();
		m_CurrentIdx++;
		// Pass amount to entt
		u = static_cast<std::underlying_type_t<entt::entity>>(size); 
	}

	void ECSManager::TESTRUN()
	{
		std::cout << "\nTEST RUNNING ECS\n====================================\n";

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";*/

		Entity test = CreateEntity("Test 1");

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";

		std::cout << "\n\nEntities IDs\n";
		for (auto& pair : m_EntityList)
		{
			std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
		}
		std::cout << "-- Storage --\n";
		for (auto&& id : GetRegistry().storage().begin()->second)
		{
			std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
		}
		std::cout << "++++++++++++\n";*/

		test->GetComponent<Transform>().m_Position.x = 19;
		std::cout << "Creating Entity, Adding, Getting and editing a value: " << test->GetComponent<Transform>().m_Position.x << std::endl;
		std::cout << "Removing Editted Component...\n";
		test->RemoveComponent<Transform>();

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";*/

		//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<Transform>().m_Scale.x << std::endl; // Will call assert in GetComponent!
		std::cout << "Default Parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(test) << "\n";
		MarkForDeletion(test);
		std::cout << "Destroyed earlier Entity...\n";
		//std::cout << "Attempting to call a deleted/destroyed Entity: " << test->GetComponent<Properties>().m_Name << std::endl; // Will not call assert in GetComponent until next loop!

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";

		std::cout << "\n\nEntities IDs\n";
		for (auto& pair : m_EntityList)
		{
			std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
		}
		std::cout << "-- Storage --\n";
		for (auto&& id : GetRegistry().storage().begin()->second)
		{
			std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
		}
		std::cout << "++++++++++++\n";*/

		std::cout << "Creating Entitys with 1 Entity with only Properties and 2 Entity with Transform and Properties...\n";
		Entity test2 = CreateEntity("test2");

		Entity allobj = CreateEntity("allobj");
		Entity allobj2 = CreateEntity("allobj2");

		std::cout << "Total Entity with Transform: " << GetEntities<Transform>().size() << "\n";
		std::cout << "Total Entity with Properties: " << GetEntities<Properties>().size() << "\n";
		std::cout << "Total Entity with Transform and Properties: " << GetEntities<Transform, Properties>().size() << "\n";

		std::cout << "Testing cloning Entity...\n";
		std::cout << "- Setting Original Entity value to 123...\n";
		Entity oriobj = CreateEntity("oriobj");
		oriobj->GetComponent<Transform>().m_Position.x = 123;
		std::cout << "- Cloning Original Entity\n";
		Entity cloneobj = CloneEntity(oriobj);
		std::cout << "- Cloned Entity value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";
		std::cout << "- Setting Original Entity value to 0...\n";
		oriobj->GetComponent<Transform>().m_Position.x = 0;
		std::cout << "- Origin Entity value is " << oriobj->GetComponent<Transform>().m_Position.x << "\n";
		std::cout << "- Cloned Entity value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";

		std::cout << "\nIterating All Available Component in ComponentManager\n";
		for (auto& comp : ComponentManager::Instance().m_Components)
		{
			std::cout << "- " << comp.second << "\n";
		}

		std::cout << "\nTesting iterating through All Entity with Properties\n";
		for (auto& go : GetEntities<Properties>())
		{
			go->GetComponent<Properties>().m_Active = true;
		}
		std::cout << "- Testing Complete\n";

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";*/

		std::cout << "\nTesting setting, getting and removing parent\n";
		Entity parentEntity = CreateEntity("ParentEntity");
		Entity childEntity = CreateEntity("ChildEntity");
		std::cout << "- Default childEntity parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(childEntity) << "\n";
		std::cout << "- childEntity address: " << childEntity << "\n";
		std::cout << "- parentEntity address: " << parentEntity << "\n";
		std::cout << "- Setting parentEntity as childEntity parent...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(childEntity, parentEntity);
		std::cout << "- New childEntity parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(childEntity) << "\n";
		std::cout << "- childEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(childEntity).size() << "\n";
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- Removing childEntity parent...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->RemoveParent(childEntity);
		std::cout << "- Removed childEntity parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(childEntity) << "\n";
		std::cout << "- childEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(childEntity).size() << "\n";
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- Setting childEntity as childEntity parent...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(childEntity, childEntity);
		std::cout << "- New childEntity parent (Ideally it would set parent as a nullptr): " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(childEntity) << "\n";
		std::cout << "- childEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(childEntity).size() << "\n";
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "\n- Adding 5 Entitys to parentEntity as children...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentEntity, test2);
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentEntity, allobj);
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentEntity, allobj2);
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentEntity, cloneobj);
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentEntity, oriobj);
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- 1 Entity removing parentEntity...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->RemoveParent(test2);
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- parentEntity abandoning 1 children...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChild(parentEntity, allobj);
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- 1 Entity Setting another parent...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(allobj2, childEntity);
		std::cout << "- childEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(childEntity).size() << "\n";
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- parentEntity abandoning all remaining children...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChildren(parentEntity);
		std::cout << "- parentEntity children size: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(parentEntity).size() << "\n";
		std::cout << "- Attempting to remove a non child in parentEntity...\n";
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChild(parentEntity, test2);
		std::cout << "- Testing Complete\n";

		std::cout << "\nTesting Parent and Child Postion, Rotation, Scale updates\n";
		Entity parentPosEntity = CreateEntity("ParentPosEntity");
		Entity childPosEntity = CreateEntity("ChildPosEntity");
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(parentPosEntity, childPosEntity);
		std::cout << "Position\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetPosition(childPosEntity, { 3, 2, 1 });
		//std::cout << "- Parent Pos: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetPosition(parentPosEntity)) << "\n";
		//std::cout << "- Child  Pos: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetPosition(childPosEntity)) << "\n";
		std::cout << "- Changing Parent Pos to (4, 5, 6)...\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetPosition(parentPosEntity, { 4, 5, 6 });
		//std::cout << "- Parent Pos: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetPosition(parentPosEntity)) << "\n";
		//std::cout << "- Child  Pos: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetPosition(childPosEntity)) << "\n";
		std::cout << "Rotation\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetRotation(childPosEntity, { 2, 4, 6 });
		//std::cout << "- Parent Rot: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetRotation(parentPosEntity)) << "\n";
		//std::cout << "- Child  Rot: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetRotation(childPosEntity)) << "\n";
		std::cout << "- Changing Parent Rot to (1, 2, 3)...\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetRotation(parentPosEntity, { 1, 2, 3 });
		//std::cout << "- Parent Rot: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetRotation(parentPosEntity)) << "\n";
		//std::cout << "- Child  Rot: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetRotation(childPosEntity)) << "\n";
		std::cout << "Scale\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetScale(childPosEntity, { 8, 2, 5 });
		//std::cout << "- Parent Scale: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetScale(parentPosEntity)) << "\n";
		//std::cout << "- Child  Scale: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetScale(childPosEntity)) << "\n";
		std::cout << "- Changing Parent Scale to (3, 1, 2)...\n";
		//ECSSystemManager::Instance().GetSystem<TransformSystem>()->SetScale(parentPosEntity, { 3, 1, 2 });
		//std::cout << "- Parent Scale: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetScale(parentPosEntity)) << "\n";
		//std::cout << "- Child  Scale: " << glm::to_string(ECSSystemManager::Instance().GetSystem<TransformSystem>()->GetScale(childPosEntity)) << "\n";

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";*/

#if false
		std::cout << "\nTesting Listener\n";
		registry.on_construct<Transform>().connect<&Transform::Init>();
		registry.on_update<Transform>().connect<&Transform::UpdateValues>();
		registry.on_destroy<Transform>().connect<&Transform::Destroy>();

		Entity listenerEntity = CreateEntity("listenerEntity");
		std::cout << "\nTesting Listening to Adding Component, should call Init\n";
		listenerEntity->AddComponent<Transform>();
		std::cout << "\nTesting Listening to Changing Component Values, should call Update\n";
		listenerEntity->GetComponent<Transform>().m_Position.x = 5; // This wont work
		listenerEntity->GetComponent<Transform>().m_Position.y = 85; // This wont work
		std::cout << "Original Value: " << listenerEntity->GetComponent<Transform>().m_Position.x << ", " << listenerEntity->GetComponent<Transform>().m_Position.y << "\n";
		// replaces the component in-place
		int newVal = 69;
		GetRegistry().patch<Transform>(listenerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = newVal; });
		GetRegistry().patch<Transform>(listenerEntity->m_Entity, &Transform::SetPosX);
		std::cout << "New Value: " << listenerEntity->GetComponent<Transform>().m_Position.x << ", " << listenerEntity->GetComponent<Transform>().m_Position.y << "\n";
		std::cout << "\nTesting Listening to Destroying Values, should call Destroy\n";
		listenerEntity->RemoveComponent<Transform>();

		std::cout << "\nDisconnecting Listeners...\n";
		registry.on_construct<Transform>().disconnect<&Transform::Init>();
		registry.on_update<Transform>().disconnect<&Transform::UpdateValues>();
		registry.on_destroy<Transform>().disconnect<&Transform::Destroy>();

		std::cout << "- Testing complete!\n";
#endif


#if false
		std::cout << "\nTesting observer noticing if any Transform change\n";
		entt::observer existingObserver{ registry, entt::collector.group<Transform>() };
		entt::observer updatedObserver{ registry, entt::collector.update<Transform>() };
		std::cout << "- Adding Entity for observer to observe...\n";
		Entity observerEntity = CreateEntity("ObserverEntity");
		observerEntity->AddComponent<Transform>();

		std::cout << "Existing Transform Observer Size: " << existingObserver.size() << "\n";

		std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";

		std::cout << "\n- Testing if changing variables manually would affect\n";
		observerEntity->GetComponent<Transform>().m_Position.x = 5;
		std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
		std::cout << "-- If size is same, it did not update\n";

		std::cout << "\n- Testing if changing variables using patch in entt would affect\n";
		GetRegistry().patch<Transform>(observerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = 5; });
		std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
		std::cout << "-- If size is same, it did not update\n";

		std::cout << "\n- Testing if changing variables using patch in entt on the same Entity would cause dups\n";
		GetRegistry().patch<Transform>(observerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = 0; });
		std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
		std::cout << "-- If size is same, it did not dup\n";

		std::cout << "\n- Clearing observers size... (REMEMBER TO DISCONNECT FROM REGISTRY OR IT WILL CRASH ON SHUTDOWN!)\n";
		existingObserver.clear();
		updatedObserver.clear();
		existingObserver.disconnect();
		updatedObserver.disconnect();
		std::cout << "- Testing complete!\n";
#endif

		std::cout << "\nTrying out snapshot for archiving entities\n";

		std::cout << "\nOBJ SIZE: " << ECSManager::Instance().GetAllEntities().size() << "\n";
		for (Entity& obj : ECSManager::Instance().GetAllEntities())
		{
			std::cout << "-" << static_cast<ENTTID>(obj->m_Entity) << "|" << obj->GetComponent<Properties>().m_Name << " | " << obj->GetComponent<Properties>().m_Active << "\n";
			std::cout << "-- Parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj) << ": " << (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj) ? ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj)->GetComponent<Properties>().m_Name : "NONE") << "\n";
			std::cout << "-- Children: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(obj).size() << "\n";
			for (Entity& child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(obj))
			{
				std::cout << "---" << static_cast<ENTTID>(child->m_Entity) << "|" << child->GetComponent<Properties>().m_Name << " | " << child->GetComponent<Properties>().m_Active << "\n";
			}
			std::cout << "\n";
		}

		std::cout << "- Archiving to Output: " << GetEntities<Properties>().size() << "...\n";
		std::string file = ECSManager::Instance().SaveEntities("../Scenes/Lmao.json");

		ECSManager::Instance().DestroyAll();

		std::cout << "\nOBJ SIZE: " << ECSManager::Instance().GetAllEntities().size() << "\n";
		std::cout << "- Total Objects: " << GetEntities<Properties>().size() << "...\n";

		std::cout << std::endl;
		std::cout << "- Loading from input: " << GetEntities<Properties>().size() << "...\n";
		ECSManager::Instance().LoadEntities("../Scenes/Lmao.json");

		std::cout << "\nOBJ SIZE: " << ECSManager::Instance().GetAllEntities().size() << "\n";
		for (Entity& obj : ECSManager::Instance().GetAllEntities())
		{
			std::cout << "-" << static_cast<ENTTID>(obj->m_Entity) << "|" << obj->GetComponent<Properties>().m_Name << " | " << obj->GetComponent<Properties>().m_Active << "\n";
			std::cout << "-- Parent: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj) << ": " << (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj) ? ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(obj)->GetComponent<Properties>().m_Name : "NONE") << "\n";
			std::cout << "-- Children: " << ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(obj).size() << "\n";
			for (Entity& child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(obj))
			{
				std::cout << "---" << static_cast<ENTTID>(child->m_Entity) << "|" << child->GetComponent<Properties>().m_Name << " | " << child->GetComponent<Properties>().m_Active << "\n";
			}
			std::cout << "\n";
		}

		//ECSOutputArchive str{};
		//entt::exclude_t<Undeployed> u;
		//const auto view = registry.view<Undeployed>();
		//int i{};
		//for (auto& obj : view)
		//{
		//	++i;
		//}
		//std::cout << ">>>> " << i << "\n";
		//// Destroys all undeployed entities
		//MemoryManager::Instance().ClearUndeployed();
		////registry.destroy(view.begin(), view.end());
		//i = 0;
		//for (auto& obj : registry.view<Undeployed>())
		//{
		//	++i;
		//}
		//std::cout << ">>>> " << i << "\n";

		//entt::snapshot snapshot{ GetRegistry() };
		//// Serialize all entities and components
		//snapshot.entities(str).component<>(str);
		//snapshot.component<>(str);

		std::cout << "\n\nEntities IDs\n";
		for (auto& pair : m_EntityList)
		{
			std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
		}
		std::cout << "-- Storage --\n";
		for (auto&& id : GetRegistry().storage().begin()->second)
		{
			std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
		}
		std::cout << "++++++++++++\n";

		std::cout << "\n- Destroy All...\n";
		DestroyAll();
		std::cout << "- Current: " << GetEntities<Properties>().size() << "...\n";

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";

		std::cout << "\n\nEntities IDs\n";
		for (auto& pair : m_EntityList)
		{
			std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
		}
		std::cout << "-- Storage --\n";
		for (auto&& id : GetRegistry().storage().begin()->second)
		{
			std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
		}
		std::cout << "++++++++++++\n";*/
		/*ECSInputArchive instr{};
		entt::snapshot_loader snapshotLoader{ GetRegistry() };
		snapshotLoader.entities(instr);*/
		//snapshotLoader.component<>(str);

#if false
		// This is for how to get entt::hashed_string
		using namespace entt::literals;

		entt::hashed_string active_hs{ "m_Active"_hs };
		entt::meta<Properties>()
			.type(entt::type_hash<Properties>::value())
			.data<&Properties::m_Active>(active_hs);

		std::cout << "\nTesting if can compile with LIONart Properties\n";
		Entity INSPECT{ ECSManager::Instance().CreateEntity("INSPECTENT")};

		std::vector<std::pair<std::string, property::base*>> components{ ECSManager::Instance().GetAllInspectableComponents(INSPECT) };

		std::cout << "- Size: " << components.size() << "\n";

		//property::base& INSPECTPROP{ INSPECT->GetComponent<Properties>() };

		std::vector< std::pair<std::string, std::vector<property::entry>>> LList;
		for (size_t i{}; i < components.size(); ++i)
		{
			property::base& INSPECTPROP { *components[i].second };
			std::vector<property::entry> List;
			property::SerializeEnum(INSPECTPROP, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
				{
					// If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
					assert(Flags.m_isScope == false || PropertyName.back() == ']');
					List.push_back(property::entry { PropertyName, Data });
				});
			LList.push_back({ components[i].first, List });
		}

		std::cout << "Original Name: " << INSPECT->GetName() << "\n";
		std::cout << "---------------\n";
		for (auto& List : LList)
		{
			std::cout << List.first << "\n";
			for (auto& [Name, Data] : List.second)
			{
				//std::cout << Name.c_str();
				std::string NameStr = Name.substr(Name.find_last_of("/") + 1);
				std::cout << NameStr;

				std::visit([&](auto&& Value)
					{
						using T = std::decay_t<decltype(Value)>;

						if constexpr (std::is_same_v<T, int>)
						{
							printf("\t int    (%d)", Value);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							printf("\t float  (%f)", Value);
						}
						else if constexpr (std::is_same_v<T, bool>)
						{
							printf("\t bool   (%s)", Value ? "true" : "false");
							// TO DELETE WE CAN CHANGE DATA THIS WAY
							Data = false;
						}
						else if constexpr (std::is_same_v<T, string_t>)
						{
							printf("\t string (%s)", Value.c_str());
							// TO DELETE WE CAN CHANGE DATA THIS WAY
							Data = "CHANGEDNAME";
						}
						else if constexpr (std::is_same_v<T, glm::vec3>)
						{
							printf("\t glm::vec3   (%f, %f)", Value[0], Value[1], Value[2]);
						}
						else if constexpr (std::is_same_v<T, resource_ref>)
						{
							//printf("\t glm::vec3   (%f, %f)", Value[0], Value[1], Value[2]);
						}
						else static_assert(always_false<T>::value, "We are not covering all the cases!");
					}
				, Data);

				std::cout << std::endl;

			}
		}
		std::cout << "---------------\n";

		std::cout << "---------------\n";
		for (auto& List : LList)
		{
			std::cout << List.first << "\n";
			for (const auto& [Name, Data] : List.second)
			{
				//std::cout << Name;
				std::string NameStr = Name.substr(Name.find_last_of("/") + 1);
				std::cout << NameStr;

				std::visit([&](auto&& Value)
					{
						using T = std::decay_t<decltype(Value)>;

						if constexpr (std::is_same_v<T, int>)
						{
							printf("\t int    (%d)", Value);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							printf("\t float  (%f)", Value);
						}
						else if constexpr (std::is_same_v<T, bool>)
						{
							printf("\t bool   (%s)", Value ? "true" : "false");
						}
						else if constexpr (std::is_same_v<T, string_t>)
						{
							printf("\t string (%s)", Value.c_str());
						}
						else if constexpr (std::is_same_v<T, glm::vec3>)
						{
							printf("\t glm::vec3   (%f, %f)", Value[0], Value[1], Value[2]);
						}
						else if constexpr (std::is_same_v<T, resource_ref>)
						{
							//printf("\t glm::vec3   (%f, %f)", Value[0], Value[1], Value[2]);
						}
						else static_assert(always_false<T>::value, "We are not covering all the cases!");
					}
				, Data);

				std::cout << std::endl;

			}
		}
		std::cout << "---------------\n";

		std::cout << "Copying list back to entity...\n";
		for (size_t i{}; i < components.size(); ++i)
		{
			property::base& INSPECTPROP { *components[i].second };
			std::vector<property::entry> List{ LList[i].second };
			for (const auto& [Name, Data] : List)
			{
				// Copy to INSPECTPROP
				property::set(INSPECTPROP, Name.c_str(), Data);
			}
		}


		std::cout << "New Name: " << INSPECT->GetName() << "\n";
#endif
		// NOT WORTH USING FOR NOW
		//void* pBase = &INSPECTPROP;
		//property::DisplayEnum(INSPECT->GetComponent<Properties>().getPropertyVTable(), pBase, [&](std::string_view PropertyName, property::data&& Data, const property::table& Table, std::size_t Index, property::flags::type Flags)
		//	{
		//		//C->m_List.push_back(std::make_unique<entry>(std::string{ PropertyName }, Data, &Table.m_pEntry[Index], Flags));
		//	});

		std::cout << "\n\nECSMANAGER PRIVATE FUNCTION!!!\n";
		std::cout << "- Testing if calling this function can add component...\n";
		std::cout << "Creating Entity w/o FEL\n";
		Entity felENT = CreateEntity("FELENT");
		std::cout << "- Does FELENT have FEL? >" << felENT->HasComponent<FEL>() << "\n";
		std::cout << "- Adding using m_AddCompFunctions...\n";
		m_AddCompFunctions[ComponentManager::Instance().GetComponentName<FEL>()](felENT);
		std::cout << "- Does FELENT have FEL? >" << felENT->HasComponent<FEL>() << "\n";

		std::cout << "\nDestroying all " << GetEntities<Properties>().size() << "  test objects...\n";
		DestroyAll();
		std::cout << "- Remaining: " << GetEntities<Properties>().size() << " | Successfully cleared: " << (GetEntities<Properties>().empty() ? "true" : "false") << "\n";

		std::cout << "\nCreating New Scene...\n";
		SceneManager::Instance().NewScene();

		std::cout << "\nCreating Entity to prefab\n";
		Entity prefabEnt = ECSManager::Instance().CreateEntity("Prefab Entity");
		prefabEnt->AddComponent<FAKEFEL>().fakeValue = "uwu";

		std::cout << "- Attempting to save prefab " << prefabEnt->GetName() << "\n";
		PrefabSystem* prefabSystem{ ECSSystemManager::Instance().GetSystem<PrefabSystem>() };
		std::string prefabEntGUID = prefabSystem->SavePrefabEntity(prefabEnt);
		std::cout << "-- Succesfully saved with prefab guid of " << prefabEntGUID << "\n";
		std::cout << "- Attempting to create prefab " << prefabEnt->GetName() << " instance\n";
		Entity prefabEntInstance = prefabSystem->CreatePrefabEntityInstance(prefabEntGUID);
		std::cout << "-- Succesfully created prefab instance named " << prefabEntInstance->GetName() << "\n";
		std::cout << "> prefabEnt: " << prefabEnt->GetComponent<Prefabing>().m_Instances.size() << "\n";
		std::cout << "> prefabEntInstance: " << prefabEntInstance->GetComponent<Prefabing>().m_Instances.size() << "\n";

		/*SceneManager::Instance().SaveSceneAs("../Scenes/TESTING.json");
		SceneManager::Instance().LoadScene("../Scenes/TESTING.json");

		for (auto& ent : GetEntities<Prefabing>())
		{
			std::cout << ent->GetName() << "|" << ent->GetComponent<Prefabing>().m_PrefabGUID << "\n";
		}*/

		/*Entity entp1{ ECSManager::Instance().CreateEntity("entp1")};
		entp1->AddComponent<FEL>().tobeignored = "owo";
		FEL p1{ entp1->GetComponent<FEL>() };
		std::cout << entp1->GetComponent<FEL>().tobeignored << " ?<\n";
		entp1->RemoveComponent<FEL>();
		std::cout << entp1->AddComponent<FEL>().tobeignored << " <<\n";
		entp1->GetComponent<FEL>() = p1;
		std::cout << entp1->GetComponent<FEL>().tobeignored << " ?<\n";*/

		std::cout << "\nDestroying all " << GetEntities<Properties>().size() << "  test objects...\n";
		DestroyAll();
		std::cout << "- Remaining: " << GetEntities<Properties>().size() << " | Successfully cleared: " << (GetEntities<Properties>().empty() ? "true" : "false") << "\n";
		std::cout << "\nCreating New Scene...\n";
		SceneManager::Instance().NewScene();

		std::cout << "====================================\n\n";
	}

	void ECSManager::STRESSTEST()
	{
		std::cout << "STRESS TEST ECS\n====================================\n";
		for (int i{}; i < 2500; ++i)
		{
			Entity ent{ ECSManager::Instance().CreateEntity() };
			ent->AddComponent<MeshRenderer>();
		}
	}
}