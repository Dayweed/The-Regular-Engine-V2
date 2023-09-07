#include "pch.h"
#include "SelectionManager.h"

namespace TRE
{
	SelectionManager::SelectionManager()
	{

	}

	SelectionManager::~SelectionManager()
	{

	}

	bool SelectionManager::IsEntitySelected(Entity& EntityObject)
	{
		if (EntityObject->GetName() == m_SelectedEntity->GetName()) return true;
		return false;
	}

	Entity& SelectionManager::GetSelectedEntity()
	{
		return m_SelectedEntity;
	}

	void SelectionManager::SelectEntity(Entity& EntityObject)
	{
		m_SelectedEntity = EntityObject;
	}
}