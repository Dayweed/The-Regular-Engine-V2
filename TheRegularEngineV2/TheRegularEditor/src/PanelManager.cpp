#include "pch.h"
#include "PanelManager.h"

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

	template <typename T>
	std::shared_ptr<Panel> PanelManager::operator=(std::shared_ptr<T> data)
	{
		return data;
	}

	//render (show panel)
	//shutdown (delete memory)
	void PanelManager::Init()
	{
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