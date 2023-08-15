#include "pch.h"
#include "ECS.h"

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

	void ECSManager::DestroyAll()
	{
		for (GO obj : m_GOList)
		{
			obj->AbandonChildren();
		}
		registry.clear();
	}

	GO ECSManager::CreateGO(std::string name)
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->m_Entity = registry.create();
		m_GOList.emplace_back(obj);
		obj->AddComponent<Properties>().m_Name = name;
		return obj;
	}

	void ECSManager::DestroyGO(GO& object)
	{
		// Remove from m_GOList
		auto it = std::find_if(m_GOList.begin(), m_GOList.end(), [&](GO& go) { return go.get() == object.get(); });
		if (it != m_GOList.end())
		{
			m_GOList.erase(it);
		}
		object->AbandonChildren();
		// Release all components and entity itself
		registry.destroy(object->m_Entity);
		// Free unique ptr from the object
		object.reset();
	}

	GO ECSManager::CloneGO(GO& object, std::string name)
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->m_Entity = registry.create();
		m_GOList.emplace_back(obj);
		// Clone each component of the object into the clone
		for (auto&& curr : registry.storage())
		{
			std::cout << "A Component Type " << curr.first << "\n";
			if (auto& storage = curr.second; storage.contains(object->m_Entity))
			{
				std::cout << "	Storage of entities with mentioned component\n";
				std::cout << "	Size of Storage: " << storage.size() << "\n";
				std::cout << "	Cloning Component...\n";
				storage.emplace(obj->m_Entity, storage.get(object->m_Entity));
			}
		}
		// Change Name
		obj->GetComponent<Properties>().m_Name = name;
		// Return clone
		return obj;
	}

	GO GameObject::GetThis()
	{
		return shared_from_this();
	}

	void GameObject::SetParent(GO parent)
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

	GO GameObject::GetParent()
	{
		return m_Parent;
	}

	void GameObject::RemoveParent()
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

	void GameObject::AddChild(GO child)
	{
		child->SetParent(GetThis());
	}

	std::vector<GO> GameObject::GetChildren()
	{
		return m_Children;
	}

	void GameObject::AbandonChild(GO child)
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

	void GameObject::AbandonChildren()
	{
		for (int i{ static_cast<int>(m_Children.size()) - 1 }; i >= 0; --i)
		{
			AbandonChild(m_Children[i]);
		}
		m_Children.clear();
	}
}