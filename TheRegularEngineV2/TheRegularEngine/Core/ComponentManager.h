#pragma once
#include "pch.h"

namespace TRE
{
	// Creating a based class Component to associate what classes/structs are a component
	// Otherwise as of now it would be empty
	class Component
	{
	};

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
		void RegisterComponent(std::string name)
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Components.find(hashcode) == m_Components.end()); //Make sure don't double register

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
		std::string GetComponentName()
		{
			auto hashcode = typeid(T).hash_code();
			if (HasComponent<T>())
			{
				return m_Components[hashcode].get();
			}

			//Can add assert here to tell user they getting a non registered component
			return "[ERROR!] Non Registered Component";
		}

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		ComponentManager() {};
		ComponentManager(ComponentManager const&) = delete;
		void operator=(ComponentManager const&) = delete;
		void* operator new(size_t) = delete;

		std::map<size_t, std::string> m_Components;
	};
	static ComponentManager* _component_manager{ &ComponentManager::Instance() };
}