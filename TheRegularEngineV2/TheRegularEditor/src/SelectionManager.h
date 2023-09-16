#pragma once
#include "pch.h"
#include "TREIncludes.h"

namespace TRE
{
	class SelectionManager
	{
		public:
			SelectionManager();
			~SelectionManager();
			bool IsEntitySelected(Entity& EntityObject);
			Entity& GetSelectedEntity();
			std::vector<std::pair<std::string, property::base*>>& GetSelectedEntityComponents();
			std::vector< std::pair<std::string, std::vector<property::entry>>>& GetSelectedEntityProperty();
			void SelectEntity(Entity& EntityObject);

		private:
			Entity m_SelectedEntity;
			std::vector<std::pair<std::string, property::base*>> m_SelectedEntityInspectableComp;
			std::vector< std::pair<std::string, std::vector<property::entry>>> m_SelectedEntityPropTable;
	};
}