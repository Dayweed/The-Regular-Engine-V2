#include "pch.h"

#include "Parent.h"
#include "Core/Logger.h"

namespace TRE
{
	void ParentingSystem::Update()
	{

	}

	void ParentingSystem::OnDestroyGO()
	{
		for (Entity& object : ECSManager::Instance().GetEntities<Removal>())
		{
			AbandonChildren(object);
		}
	}

	void ParentingSystem::Shutdown()
	{
		for (Entity& object : ECSManager::Instance().GetAllEntities())
		{
			AbandonChildren(object);
		}
	}

	void ParentingSystem::SetParent(Entity child, Entity parent)
	{
		// Tell existing parent to abandon this
		if (ECSManager::Instance().FindEntity(child->GetComponent<Parenting>().m_Parent))
		{
			AbandonChild(ECSManager::Instance().FindEntity(child->GetComponent<Parenting>().m_Parent), child);
		}
		// Ensure self can't be parent or parent is one of its children
		child->GetComponent<Parenting>().m_Parent = (parent->GetGUID() != child->GetGUID() && std::find(child->GetComponent<Parenting>().m_Children.begin(), child->GetComponent<Parenting>().m_Children.end(), ECSManager::Instance().FindEntityID(parent)) == child->GetComponent<Parenting>().m_Children.end()) ? ECSManager::Instance().FindEntityID(parent) : "";

		// Add this as parent child if valid
		if (child->GetComponent<Parenting>().m_Parent != "" && std::find(parent->GetComponent<Parenting>().m_Children.begin(), parent->GetComponent<Parenting>().m_Children.end(), ECSManager::Instance().FindEntityID(child)) == parent->GetComponent<Parenting>().m_Children.end())
		{
			parent->GetComponent<Parenting>().m_Children.emplace_back(ECSManager::Instance().FindEntityID(child));
		}
	}

	Entity ParentingSystem::GetParent(Entity child)
	{
		return ECSManager::Instance().FindEntity(child->GetComponent<Parenting>().m_Parent);
	}

	void ParentingSystem::RemoveParent(Entity child)
	{
		std::string parentID{ child->GetComponent<Parenting>().m_Parent };
		if (parentID != "")
		{
			auto it = std::find(ECSManager::Instance().FindEntity(parentID)->GetComponent<Parenting>().m_Children.begin(), ECSManager::Instance().FindEntity(parentID)->GetComponent<Parenting>().m_Children.end(), ECSManager::Instance().FindEntityID(child));
			if (it != ECSManager::Instance().FindEntity(parentID)->GetComponent<Parenting>().m_Children.end())
			{
				ECSManager::Instance().FindEntity(parentID)->GetComponent<Parenting>().m_Children.erase(it);
			}
		}
		child->GetComponent<Parenting>().m_Parent = "";
	}

	void ParentingSystem::AddChild(Entity parent, Entity child)
	{
		SetParent(child, parent);
	}

	std::vector<Entity> ParentingSystem::GetChildren(Entity parent)
	{
		std::vector<Entity> children;
		for (std::string id : parent->GetComponent<Parenting>().m_Children)
		{
			Entity child{ ECSManager::Instance().FindEntity(id) };
			if (child)
			{
				children.emplace_back(child);
			}
		}

		return children;
	}

	void ParentingSystem::AbandonChild(Entity parent, Entity child)
	{
		if (GetParent(child) && ECSManager::Instance().FindEntityID(GetParent(child)) == ECSManager::Instance().FindEntityID(parent))
		{
			RemoveParent(child);
			auto it = std::find(parent->GetComponent<Parenting>().m_Children.begin(), parent->GetComponent<Parenting>().m_Children.end(), ECSManager::Instance().FindEntityID(child));
			if (it != parent->GetComponent<Parenting>().m_Children.end())
			{
				parent->GetComponent<Parenting>().m_Children.erase(it);
			}
		}
		else
		{
			std::string funcName{ __FUNCTION__ };
			std::string childName{ child->GetName() };
			std::string parentName{ parent->GetName() };
			TRE_CORE_WARN("[" + funcName + "] " + parentName + " does not have the child " + childName + "...");
		}
	}

	void ParentingSystem::AbandonChildren(Entity parent)
	{
		for (int i{ static_cast<int>(parent->GetComponent<Parenting>().m_Children.size()) - 1 }; i >= 0; --i)
		{
			AbandonChild(parent, ECSManager::Instance().FindEntity(parent->GetComponent<Parenting>().m_Children[i]));
		}
		parent->GetComponent<Parenting>().m_Children.clear();
	}
}