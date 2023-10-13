#include "pch.h"

#include "Parent.h"
#include "Transform.h"
#include "Logger.h"

namespace TRE
{
	void ParentingSystem::Update()
	{
		for (Entity& object : ECSManager::Instance().GetEntities<Transform, Parenting>())
		{
			Transform& transform{ object->GetComponent<Transform>() };
			if (transform.m_IsDirty)
			{
				UpdateChildTransform(object);
			}
		}
	}

	void ParentingSystem::OnReset()
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
			child->GetComponent<Transform>().UpdateLocalMatrix(parent);
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
			Parenting& parenting{ ECSManager::Instance().FindEntity(parentID)->GetComponent<Parenting>() };
			auto it = std::find(parenting.m_Children.begin(), parenting.m_Children.end(), ECSManager::Instance().FindEntityID(child));
			if (it != parenting.m_Children.end())
			{
				parenting.m_Children.erase(it);
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
			else
			{
				std::string funcName{ __FUNCTION__ };
				TRE_WARN("[" + funcName + "] Unable to find child id (" + id + "), skipping...");
			}
		}

		return children;
	}

	void ParentingSystem::AbandonChild(Entity parent, Entity child)
	{
		Parenting& parenting{ parent->GetComponent<Parenting>() };
		if (GetParent(child) && ECSManager::Instance().FindEntityID(GetParent(child)) == ECSManager::Instance().FindEntityID(parent))
		{
			RemoveParent(child);
			auto it = std::find(parenting.m_Children.begin(), parenting.m_Children.end(), ECSManager::Instance().FindEntityID(child));
			if (it != parenting.m_Children.end())
			{
				parenting.m_Children.erase(it);
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
		Parenting& parenting{ parent->GetComponent<Parenting>() };
		for (int i{ static_cast<int>(parenting.m_Children.size()) - 1 }; i >= 0; --i)
		{
			Entity child{ ECSManager::Instance().FindEntity(parenting.m_Children[i]) };
			if (child)
			{
				AbandonChild(parent, child);
			}
			else
			{
				std::string funcName{ __FUNCTION__ };
				TRE_WARN("[" + funcName + "] Unable to find id " + parenting.m_Children[i] + " in parent (" + parent->GetName() + ") [" + parent->GetGUID() + "]! Ignoring...");
			}
		}
		parenting.m_Children.clear();
	}

	void ParentingSystem::UpdateChildTransform(Entity parent)
	{
		Transform& parentTransform = parent->GetComponent<Transform>();
		for (Entity& child : GetChildren(parent))
		{
			Transform& childTransform = child->GetComponent<Transform>();
			const glm::mat4 newChildXform = parentTransform.m_WorldXform * childTransform.m_LocalXform;
			childTransform.DecomposeWorldMatrix(newChildXform);
			childTransform.m_IsDirty = true;
			Parenting& deeperChild{ child->GetComponent<Parenting>() };
			if (deeperChild.m_Children.size() > 0)
			{
				UpdateChildTransform(child);
			}
		}
	}
}