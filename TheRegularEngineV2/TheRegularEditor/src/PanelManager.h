#pragma once
#include "TREIncludes.h"
#include "EditorSystem.h"

namespace TRE
{
	class PanelManager
	{
		public:
			static PanelManager& Instance()
			{
				static PanelManager instance;
				return instance;
			}
			~PanelManager();

			template <typename T>
			void InsertPanel(std::string PanelName, T data)
			{
				std::shared_ptr<Panel> ptr1 = std::make_shared<Panel>(data);
				StorePanels.insert({ PanelName, ptr1 });
			}

			void RemovePanel(std::string PanelName);

			bool DoesPanelExist(std::string PanelName);

			std::unordered_map<std::string, std::shared_ptr<Panel>>& GetPanels();

		private:
			PanelManager();
			PanelManager(PanelManager const&) = delete;
			void operator=(PanelManager const&) = delete;
			void* operator new(size_t) = delete;

			std::unordered_map<std::string, std::shared_ptr<Panel>> StorePanels;
	};
}