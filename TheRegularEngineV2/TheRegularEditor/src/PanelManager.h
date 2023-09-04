#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class PanelManager
	{
		public:
			//static PanelManager& Instance()
			//{
			//	static PanelManager instance;
			//	return instance;
			//}
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
			void EntityClicked(std::string EntityName);

		private:
			//PanelManager(PanelManager const&) = delete;
			//void operator=(PanelManager const&) = delete;
			//void* operator new(size_t) = delete;

			std::unordered_map<std::string, std::shared_ptr<Panel>> m_StorePanels;
	};
}