#include "PanelManager.h"

namespace TRE
{
	PanelManager::PanelManager()
	{
		StorePanels.clear();
	}

	PanelManager::~PanelManager()
	{
		StorePanels.clear();
	}

	//template <typename T>
	//void PanelManager::InsertPanel(std::string PanelName, T data)
	//{
	//	std::shared_ptr<Panel> ptr1 = std::make_shared<Panel>(data);
	//	StorePanels.insert({ PanelName, ptr1 });
	//}

	void PanelManager::RemovePanel(std::string PanelName)
	{
		StorePanels.erase(PanelName);
	}

	bool PanelManager::DoesPanelExist(std::string PanelName)
	{
		if (StorePanels.find(PanelName) == StorePanels.end()) return false;
		return true;
	}

	std::unordered_map<std::string, std::shared_ptr<Panel>>& PanelManager::GetPanels()
	{
		return StorePanels;
	}
}