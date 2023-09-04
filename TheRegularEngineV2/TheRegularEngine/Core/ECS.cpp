#include "pch.h"
#include "ECS.h"
#include "MemoryManager.h"
#include "Core/Logger.h"

namespace TRE
{
	ECSManager& ECSManager::Instance()
	{
		static ECSManager instance;
		return instance;
	}

	entt::registry& ECSManager::GetRegistry()
	{
		return registry;
	}

	void ECSManager::DeleteRemovalEntities()
	{
		for (Entity& object : GetEntities<Removal>())
		{
			// Remove from m_EntityList
			m_EntityList.erase(m_EntityList.find(static_cast<Entity_ID>(object->m_Entity)));
			MemoryManager::Instance().ReleaseDeployedEntity(static_cast<Entity_ID>(object->m_Entity));
			//object->AbandonChildren();
			//// Release all components and entity itself
			//registry.destroy(object->m_Entity);
			//// Free unique ptr from the object
			//object.reset();
		}
	}

	void ECSManager::DestroyAll()
	{
		for (auto& pair : m_EntityList)
		{
			pair.second->AbandonChildren();
			MarkForDeletion(pair.second);
		}
		DeleteRemovalEntities();
	}

	Entity ECSManager::CreateEntity(std::string name)
	{
		/*Entity obj{ std::make_shared<Ent>() };
		obj->m_Entity = registry.create();
		m_EntityList.emplace(static_cast<Entity_ID>(obj->m_Entity), obj);
		obj->AddComponent<Properties>().m_Name = name;
		obj->AddComponent<Transform>();
		return obj;*/
		Entity obj{ MemoryManager::Instance().GetUndeployedEntity() };
		m_EntityList.emplace(static_cast<Entity_ID>(obj->m_Entity), obj);
		obj->GetComponent<Properties>().m_Name = name;
		return obj;
	}

	void ECSManager::MarkForDeletion(Entity& object)
	{
		object->AddComponent<Removal>();
		return;
	}

	Entity ECSManager::CloneEntity(Entity& object, std::string name)
	{
		Entity obj{ MemoryManager::Instance().GetUndeployedEntity() };
		// Remove all components in one entity
		for (auto&& elem : ECSManager::Instance().GetRegistry().storage()) {
			elem.second.remove(obj->m_Entity);
		}
		m_EntityList.emplace(static_cast<Entity_ID>(obj->m_Entity), obj);
		// Clone each component of the object into the clone
		for (auto&& curr : registry.storage())
		{
			TRE_CORE_INFO("A Component Type {0}", curr.first);
			if (auto& storage = curr.second; storage.contains(object->m_Entity))
			{
				TRE_CORE_INFO("Storage of entities with mentioned component");
				TRE_CORE_INFO("Size of Storage: {0}", storage.size());
				TRE_CORE_INFO("Cloning Component...");
				storage.emplace(obj->m_Entity, storage.get(object->m_Entity));
			}
		}
		// Change Name
		obj->GetComponent<Properties>().m_Name = name;
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

	void ECSManager::SaveEntities(std::string filePath)
	{
		// Set up document
		ECSOutputArchive arc { "Testing.json" };

		// Destroys all undeployed entities
		MemoryManager::Instance().ClearUndeployed();

		entt::snapshot snapshot{ GetRegistry() };
		// Serialize all entities and components
		snapshot.entities(arc).component<>(arc);

		for (Entity& ent : GetEntities<Properties>())
		{
			//object.insertValue("GO_objectName", ent, allocator);
		}
	}

	void ECSManager::LoadEntities(std::string filePath)
	{

	}

	Entity Ent::GetThis()
	{
		return shared_from_this();
	}

	void Ent::SetParent(Entity parent)
	{
		// Tell existing parent to abandon this
		if (m_Parent)
		{
			m_Parent->AbandonChild(GetThis());
		}
		// Ensure self can't be parent or parent is one of its children
		m_Parent = (parent.get() != this && std::find(m_Children.begin(), m_Children.end(), parent) == m_Children.end()) ? parent : nullptr;
		// Add this as parent child
		if (m_Parent && std::find(m_Parent->m_Children.begin(), m_Parent->m_Children.end(), GetThis()) == m_Parent->m_Children.end())
		{
			m_Parent->m_Children.emplace_back(GetThis());
		}
	}

	Entity Ent::GetParent()
	{
		return m_Parent;
	}

	void Ent::RemoveParent()
	{
		if (m_Parent)
		{
			auto it = std::find(m_Parent->m_Children.begin(), m_Parent->m_Children.end(), shared_from_this());
			if (it != m_Parent->m_Children.end())
			{
				m_Parent->m_Children.erase(it);
			}
		}
		m_Parent = nullptr;
	}

	void Ent::AddChild(Entity child)
	{
		child->SetParent(GetThis());
	}

	std::vector<Entity> Ent::GetChildren()
	{
		return m_Children;
	}

	void Ent::AbandonChild(Entity child)
	{
		if (child->GetParent().get() == this)
		{
			child->RemoveParent();
			auto it = std::find(m_Children.begin(), m_Children.end(), child);
			if (it != m_Children.end())
			{
				m_Children.erase(it);
			}
		}
	}

	void Ent::AbandonChildren()
	{
		for (int i{ static_cast<int>(m_Children.size()) - 1 }; i >= 0; --i)
		{
			AbandonChild(m_Children[i]);
		}
		m_Children.clear();
	}

	ECSOutputArchive::ECSOutputArchive(std::string filePath) : m_FilePath(filePath)
	{
		m_Doc.StartArray();
	}

	void ECSOutputArchive::operator()(entt::entity ent)
	{
		if (ECSManager::Instance().GetRegistry().valid(ent))
		{
			std::cout << static_cast<std::underlying_type_t<entt::entity>>(ent) << " - ";
		}
	}

	void ECSOutputArchive::operator()(std::underlying_type_t<entt::entity> u)
	{
		//m_Doc(u);
		std::cout << u << ";";
	}

	void ECSOutputArchive::Close()
	{
		std::ofstream file(m_FilePath.c_str());
		rapidjson::OStreamWrapper osw(file);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		m_Doc.Accept(writer);
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

		test->AddComponent<Transform>().m_Position.x = 19;
		std::cout << "Creating Entity, Adding, Getting and editing a value: " << test->GetComponent<Transform>().m_Position.x << std::endl;
		std::cout << "Removing Editted Component...\n";
		test->RemoveComponent<Transform>();

		/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
		for (auto&& storage : GetRegistry().storage())
			std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
		std::cout << "-------\n";*/

		//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<Transform>().m_Scale.x << std::endl; // Will call assert in GetComponent!
		std::cout << "Default Parent: " << test->GetParent() << "\n";
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
		oriobj->AddComponent<Transform>().m_Position.x = 123;
		std::cout << "- Cloning Original Entity\n";
		Entity cloneobj = CloneEntity(oriobj);
		std::cout << "- Cloned Entity value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";
		std::cout << "- Setting Original Entity value to 0...\n";
		oriobj->GetComponent<Transform>().m_Position.x = 0;
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
		Entity parentEntity = CreateEntity();
		Entity childEntity = CreateEntity();
		std::cout << "- Default childEntity parent: " << childEntity->GetParent() << "\n";
		std::cout << "- childEntity address: " << childEntity << "\n";
		std::cout << "- parentEntity address: " << parentEntity << "\n";
		std::cout << "- Setting parentEntity as childEntity parent...\n";
		childEntity->SetParent(parentEntity);
		std::cout << "- New childEntity parent: " << childEntity->GetParent() << "\n";
		std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- Removing childEntity parent...\n";
		childEntity->RemoveParent();
		std::cout << "- Removed childEntity parent: " << childEntity->GetParent() << "\n";
		std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- Setting childEntity as childEntity parent...\n";
		childEntity->SetParent(childEntity);
		std::cout << "- New childEntity parent (Ideally it would set parent as a nullptr): " << childEntity->GetParent() << "\n";
		std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "\n- Adding 5 Entitys to parentEntity as children...\n";
		parentEntity->AddChild(test2);
		parentEntity->AddChild(allobj);
		parentEntity->AddChild(allobj2);
		parentEntity->AddChild(cloneobj);
		parentEntity->AddChild(oriobj);
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- 1 Entity removing parentEntity...\n";
		test2->RemoveParent();
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- parentEntity abandoning 1 children...\n";
		parentEntity->AbandonChild(allobj);
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- 1 Entity Setting another parent...\n";
		allobj2->SetParent(childEntity);
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- parentEntity abandoning all remaining children...\n";
		parentEntity->AbandonChildren();
		std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
		std::cout << "- Attempting to remove a non child in parentEntity...\n";
		parentEntity->AbandonChild(test2);
		std::cout << "- Testing Complete\n";

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
		//int newVal = 69;
		//GetRegistry().patch<Transform>(listenerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = newVal; });
		//GetRegistry().patch<Transform>(listenerEntity->m_Entity, &Transform::SetPosX);
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
		std::cout << "- Total Objects: " << GetEntities<Properties>().size() << "...\n";

		std::cout << "- Archiving to Output: " << GetEntities<Properties>().size() << "...\n";
		ECSManager::Instance().SaveEntities("Lmao.json");
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

		std::cout << "\nDestroying all " << GetEntities<Properties>().size() << "  test objects...\n";
		DestroyAll();
		std::cout << "- Remaining: " << GetEntities<Properties>().size() << " | Successfully cleared: " << (GetEntities<Properties>().empty() ? "true" : "false") << "\n";

		std::cout << "====================================\n\n";
	}
}