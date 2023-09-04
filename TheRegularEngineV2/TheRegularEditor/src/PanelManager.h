#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class PanelManager
	{
		public:
			PanelManager();
			~PanelManager();

			template <typename T>
			void InsertPanel(std::string PanelName)
			{
				std::shared_ptr<Panel> ptr1 = std::make_shared<T>();
				m_StorePanels.insert({ PanelName, ptr1 });
			}

			void Init();
			void Update();
			void RemovePanel(std::string PanelName);
			bool DoesPanelExist(std::string PanelName);
			std::unordered_map<std::string, std::shared_ptr<Panel>>& GetPanels();

		private:
			std::unordered_map<std::string, std::shared_ptr<Panel>> m_StorePanels;
	};
}