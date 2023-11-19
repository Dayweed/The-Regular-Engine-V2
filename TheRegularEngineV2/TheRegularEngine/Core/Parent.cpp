#include "pch.h"

#include "Parent.h"
#include "Transform.h"
#include "Logger.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace TRE
{
	void ParentingSystem::Update()
	{
		
	}

	void ParentingSystem::GameUpdate()
	{
		
	}

	void ParentingSystem::LateUpdate()
	{
		for (Entity& object : ECSManager::Instance().GetEntities<Parenting>())
		{
			//For startup
			if (Parenting& parent{ object->GetComponent<Parenting>() }; parent.m_IsDirty)
			{
				UpdateLocalData(object);

				parent.m_IsDirty = false;
			}
		}

		for (Entity& object : ECSManager::Instance().GetEntities<Parenting>())
		{
			//Update world data
			if (Transform& transform{ object->GetComponent<Transform>() }; transform.m_IsDirty && object->GetComponent<Parenting>().m_IsDirty == false)
			{
				//transform.CalculateWorldMatrix();

				//Update own local data if i have a parent
				UpdateLocalData(object);

				//Update children local data
				UpdateChildTransform(object);
			}
		}
	}

	void ParentingSystem::AfterReset()
	{

	}

	void ParentingSystem::OnDestroyEntities()
	{
		for (Entity& object : ECSManager::Instance().GetEntities<Removal>())
		{
			RemoveParent(object);
			AbandonChildren(object);
		}
	}

	void ParentingSystem::Shutdown()
	{
		for (Entity& object : ECSManager::Instance().GetAllEntities(true))
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
			UpdateChildLocalData(parent, child);
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

		child->GetComponent<Transform>().m_LocalPosition = {};
		child->GetComponent<Transform>().m_LocalRotation = {};
		child->GetComponent<Transform>().m_LocalScale = {1, 1, 1};
		child->GetComponent<Transform>().m_IsDirty = true;
	}

	void ParentingSystem::AddChild(Entity parent, Entity child)
	{
		SetParent(child, parent);
		UpdateChildLocalData(parent, child);
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

	void ParentingSystem::GetTotalEntities(int& noOfEntities, Entity object)
	{
		++noOfEntities;
		for (Entity child : GetChildren(object))
		{
			GetTotalEntities(noOfEntities, child);
		}
	}

	void ParentingSystem::UpdateChildTransform(Entity parent)
	{
		Transform& parentTransform = parent->GetComponent<Transform>();
		for (Entity& child : GetChildren(parent))
		{
			Transform& childTransform = child->GetComponent<Transform>();
			const glm::mat4 newChildXform = parentTransform.m_WorldXform * childTransform.CalculateLocalMatrix();
			childTransform.DecomposeWorldMatrix(newChildXform);
			//childTransform.m_IsDirty = true;

			if (child->GetComponent<Parenting>().m_Children.size() > 0)
			{
				UpdateChildTransform(child);
			}
		}
	}

	void ParentingSystem::UpdateChildLocalData(Entity parent, Entity child)
	{
		Transform& parentTransform = parent->GetComponent<Transform>();
		Transform& childTransform = child->GetComponent<Transform>();

		childTransform.UpdateLocalData(parentTransform);
		childTransform.m_IsDirty = true;
	}

	void ParentingSystem::UpdateLocalData(Entity current)
	{
		Transform& currentTransform = current->GetComponent<Transform>();
		//If this object has a parent, update local data
		if (current->GetComponent<Parenting>().m_Parent != "")
		{
			Transform& parentTransform = ECSManager::Instance().FindEntity(current->GetComponent<Parenting>().m_Parent)->GetComponent<Transform>();
			currentTransform.UpdateLocalData(parentTransform);
		}

		currentTransform.m_IsDirty = true;
	}
}