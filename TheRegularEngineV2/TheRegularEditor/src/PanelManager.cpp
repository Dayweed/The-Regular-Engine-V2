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
		m_StorePanels.clear();
	}

	PanelManager::~PanelManager()
	{
		m_StorePanels.clear();
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

		for (auto x : m_StorePanels)
		{
			x.second->Init();
		}
	}

	//(viewport and panels; nothing to do with rendering)
	void PanelManager::Update()
	{
		for (auto x : m_StorePanels)
		{
			x.second->Update();
		}
	}

	void PanelManager::RemovePanel(std::string PanelName)
	{
		m_StorePanels.erase(PanelName);
	}

	bool PanelManager::DoesPanelExist(std::string PanelName)
	{
		if (m_StorePanels.find(PanelName) == m_StorePanels.end()) return false;
		return true;
	}

	std::unordered_map<std::string, std::shared_ptr<Panel>>& PanelManager::GetPanels()
	{
		return m_StorePanels;
	}
}