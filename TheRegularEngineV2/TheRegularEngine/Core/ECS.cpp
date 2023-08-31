#include "pch.h"
#include "ECS.h"
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
			object->AbandonChildren();
			// Release all components and entity itself
			registry.destroy(object->m_Entity);
			// Free unique ptr from the object
			object.reset();
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
		Entity obj{ std::make_shared<Ent>() };
		obj->m_Entity = registry.create();
		m_EntityList.emplace(static_cast<uint32_t>(obj->m_Entity), obj);
		obj->AddComponent<Properties>().m_Name = name;
		obj->AddComponent<Transform>();
		return obj;
	}

	void ECSManager::MarkForDeletion(Entity& object)
	{
		object->AddComponent<Removal>();
		return;
	}

	Entity ECSManager::CloneEntity(Entity& object, std::string name)
	{
		Entity obj{ std::make_shared<Ent>() };
		obj->m_Entity = registry.create();
		m_EntityList.emplace(static_cast<uint32_t>(obj->m_Entity), obj);
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
}