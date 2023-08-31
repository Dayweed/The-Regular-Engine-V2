#include "PanelManager.h"
#include "SceneHierarchyPanel.h"
#include "ViewportPanel.h"
#include "MenuBarPanel.h"
#include "InspectorPanel.h"
#include "ContentBrowserPanel.h"
#include "ConsolePanel.h"

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

	//render (show panel)
	//shutdown (delete memory)
	void PanelManager::Init()
	{
		InsertPanel<SceneHierarchyPanel>("Scene Hierarchy");
		InsertPanel<ViewportPanel>("Viewport");
		InsertPanel<MenuBarPanel>("Menu Bar");
		InsertPanel<InspectorPanel>("Inspector");
		InsertPanel<ContentBrowserPanel>("Content Browser");
		InsertPanel<ConsolePanel>("Console");
	}

	//(viewport and panels; nothing to do with rendering)
	void PanelManager::Update()
	{
		for (auto x : StorePanels)
		{
			x.second->Update();
		}
	}

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