#pragma once
#include "pch.h"
#include "External/entt.hpp"
//#include "ECS.h"

namespace TRE
{
	class ComponentManager
	{
		friend class ECSManager;

	public:
		static ComponentManager& Instance()
		{
			static ComponentManager instance;
			return instance;
		}

		template <typename T>
		void RegisterComponent(std::string name, bool hidden = false)
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Components.find(hashcode) == m_Components.end()); //Make sure don't double register
			assert(m_HiddenComponents.find(hashcode) == m_HiddenComponents.end()); //Make sure don't double register

			if (hidden)
				m_HiddenComponents.emplace(std::piecewise_construct, std::forward_as_tuple(hashcode), std::forward_as_tuple(name));
			else
				m_Components.emplace(std::piecewise_construct, std::forward_as_tuple(hashcode), std::forward_as_tuple(name));
			// m_Components.insert({ hashcode, name });	// This works too
		}

		template <typename T>
		void DeleteComponent()
		{
			auto hashcode = typeid(T).hash_code();
			if (m_Components.find(hashcode) != m_Components.end())
			{
				m_Components.erase(hashcode);
			}
		}

		template <typename T>
		bool HasComponent()
		{
			auto hashcode = typeid(T).hash_code();
			return m_Components.find(hashcode) != m_Components.end();
		}

		template <typename T>
		bool HasHiddenComponent()
		{
			auto hashcode = typeid(T).hash_code();
			return m_HiddenComponents.find(hashcode) != m_HiddenComponents.end();
		}

		template <typename T>
		std::string GetComponentName()
		{
			auto hashcode = typeid(T).hash_code();
			if (HasComponent<T>())
			{
				return m_Components[hashcode];
			}
			
			// Check in hidden component
			if (m_HiddenComponents.find(hashcode) != m_HiddenComponents.end())
			{
				return m_HiddenComponents[hashcode];
			}

			//Can add assert here to tell user they getting a non registered component
			return "[ERROR!] Non Registered Component";
		}

		std::map<size_t, std::string> GetImguiAddComp()
		{
			return m_Components;
		}


	private:
		// Delete possible copy ctor and assignment to ensure singleton
		ComponentManager() {};
		ComponentManager(ComponentManager const&) = delete;
		void operator=(ComponentManager const&) = delete;
		void* operator new(size_t) = delete;

		std::map<size_t, std::string> m_Components;		  // This only has components to be viewed in Imgui Add Component
		std::map<size_t, std::string> m_HiddenComponents; // This is not to be exposed in Imgui
	};
}