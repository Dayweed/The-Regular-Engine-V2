#pragma once
#include "pch.h"
#include "Layer.h"

namespace TRE
{
	class LayerSystem
	{
		public:
			~LayerSystem() = default;
			LayerSystem(LayerSystem&) = delete;
			void operator=(const LayerSystem&) = delete;

			void UpdateSystems();
			void ShutdownSystems();

			void AddSystem(Layer* System);
			
			static LayerSystem* GetInstance();
			static void ShutDownLayerSystem();
		
		private:
			LayerSystem() = default;
			std::vector<Layer*> m_LayerSystems;
			static LayerSystem* m_Instance;
	};
}