#include "pch.h"
#include "LayerSystem.h"

namespace TRE
{
	LayerSystem* LayerSystem::m_Instance = nullptr;

	LayerSystem* LayerSystem::GetInstance()
	{
		if (m_Instance == nullptr)
			m_Instance = new LayerSystem();

		return m_Instance;
	}

	void LayerSystem::ShutDownLayerSystem()
	{
		if (m_Instance)
			delete m_Instance;
	}

	void LayerSystem::UpdateSystems()
	{
		for (auto System : m_LayerSystems)
		{
			System->Update();
		}
	}

	void LayerSystem::ShutdownSystems()
	{
		for (std::vector<Layer*>::reverse_iterator Layer = m_LayerSystems.rbegin(); Layer != m_LayerSystems.rend(); Layer++)
		{
			(*Layer)->Shutdown();
			delete (*Layer);
		}
	}

	void LayerSystem::AddSystem(Layer* System)
	{
		m_LayerSystems.push_back(System);
	}
}