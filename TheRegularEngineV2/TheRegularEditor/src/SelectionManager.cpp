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

	void SelectionManager::ClearSelectedEntity()
	{
		m_SelectedEntity = nullptr;
		m_SelectedEntityInspectableComp.clear();
	}

	std::vector<std::pair<std::string, property::base*>>& SelectionManager::GetSelectedEntityComponents()
	{
		return m_SelectedEntityInspectableComp;
	}

	std::vector< std::pair<std::string, std::vector<property::entry>>>& SelectionManager::GetSelectedEntityProperty()
	{
		return m_SelectedEntityPropTable;
	}

	void SelectionManager::SelectEntity(Entity& EntityObject)
	{
		if (EntityObject == nullptr) return;

		m_SelectedEntity = EntityObject;

		UpdateSelectedEntity();
	}

	void SelectionManager::UpdateSelectedEntity()
	{
		// If the EntityObject is valid, update all inspectable components
		if (ECSManager::Instance().IsValidEntity(m_SelectedEntity))
		{
			m_SelectedEntityPropTable.clear();
			m_SelectedEntityInspectableComp.clear();

			m_SelectedEntityInspectableComp = ECSManager::Instance().GetAllInspectableComponents(m_SelectedEntity);
			for (size_t i{}; i < m_SelectedEntityInspectableComp.size(); ++i)
			{
				property::base& compProp { *m_SelectedEntityInspectableComp[i].second };
				//std::cout << "what is this: " << m_SelectedEntityInspectableComp[i].first.c_str() << "\n";
				std::vector<property::entry> List;
				property::SerializeEnum(compProp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
					{
						// If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
						assert(Flags.m_isScope == false || PropertyName.back() == ']');
						(void)Flags;
						List.push_back(property::entry { PropertyName, Data });
					});
				m_SelectedEntityPropTable.push_back({ m_SelectedEntityInspectableComp[i].first, List });
			}
		}
		else
		{
			std::string funcName{ __FUNCTION__ };
			std::string selectedAddress{ std::to_string((unsigned long long)(void**) & m_SelectedEntity) };
			TRE_ERROR("[" + funcName + "] m_SelectedEntity (" + selectedAddress + ") is not valid! Clearing...");
			//assert(ECSManager::Instance().IsValidEntity(m_SelectedEntity));
			ClearSelectedEntity();
		}
	}
}