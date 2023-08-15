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
		registry.clear();
	}

	GO ECSManager::CreateGO(std::string name)
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->m_Entity = registry.create();
		GOList.emplace_back(obj);
		obj->AddComponent<Properties>().m_Name = name;
		return obj;
	}

	void ECSManager::DestroyGO(GO& object)
	{
		// Remove from GOList
		auto it = std::find_if(GOList.begin(), GOList.end(), [&](GO& go) { return go.get() == object.get(); });
		if (it != GOList.end())
		{
			GOList.erase(it);
		}
		// Release all components and entity itself
		registry.destroy(object->m_Entity);
		// Free unique ptr from the object
		object.reset();
	}

	GO ECSManager::CloneGO(GO& object, std::string name)
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->m_Entity = registry.create();
		GOList.emplace_back(obj);
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

	void GameObject::SetParent(GO parent)
	{
		// Tell existing parent to abandon this
		if (m_Parent)
		{
			m_Parent->AbandonChild(shared_from_this());
		}
		// Ensure self can't be parent or parent is one of its children
		m_Parent = (parent.get() != this && std::find(m_Children.begin(), m_Children.end(), parent) == m_Children.end()) ? parent : nullptr;
		// Add this as parent child
		if (m_Parent && std::find(m_Parent->m_Children.begin(), m_Parent->m_Children.end(), shared_from_this()) == m_Parent->m_Children.end())
		{
			m_Parent->m_Children.emplace_back(shared_from_this());
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
		child->SetParent(shared_from_this());
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