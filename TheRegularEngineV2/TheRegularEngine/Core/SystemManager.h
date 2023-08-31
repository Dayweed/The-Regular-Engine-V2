#pragma once
#include "pch.h"
#include "System.h"

namespace TRE
{
	class EditorSystemManager
	{
	public:
		static EditorSystemManager& Instance()
		{
			static EditorSystemManager instance;
			return instance;
		}

		template <typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Systems.find(hashcode) == m_Systems.end()); //Make sure don't double register

			std::shared_ptr<T> system = std::make_shared<T>();
			m_Systems.insert({ hashcode, std::move(system) });
			//m_Systems.emplace(std::piecewise_construct, std::forward_as_tuple(hashcode), std::forward_as_tuple());
			return system;
		}

		template <typename T, typename... Arguments>
		std::shared_ptr<T> RegisterSystem(Arguments&&... arg)
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Systems.find(hashcode) == m_Systems.end()); //Make sure don't double register

			std::shared_ptr<T> system = std::make_shared<T>(std::forward<Arguments>(arg)...);
			m_Systems.insert({ hashcode, std::move(system) });
			return system;
		}

		template <typename T>
		void DeleteSystem()
		{
			auto hashcode = typeid(T).hash_code();
			if (m_Systems.find(hashcode) != m_Systems.end())
			{
				m_Systems.erase(hashcode);
			}
		}

		template <typename T>
		T* GetSystem()
		{
			auto hashcode = typeid(T).hash_code();
			if (m_Systems.find(hashcode) != m_Systems.end())
			{
				return dynamic_cast<T*>(m_Systems[hashcode].get());
			}

			//Can add assert here to tell user they getting a non registered system
			return nullptr;
		}

		void UpdateSystem()
		{
			for (auto& system : m_Systems)
			{
				system.second->Update();
			}
		}

		void RenderImgui()
		{
			for (auto& system : m_Systems)
			{
				system.second->RenderImgui();
			}
		}

		void ShutdownSystem()
		{
			for (auto& system : m_Systems)
			{
				system.second->Shutdown();
			}
		}

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		EditorSystemManager() {};
		EditorSystemManager(EditorSystemManager const&) = delete;
		void operator=(EditorSystemManager const&) = delete;
		void* operator new(size_t) = delete;

		std::map<size_t, std::shared_ptr<System>> m_Systems;
	};
	static EditorSystemManager* _editor_system_manager{ &EditorSystemManager::Instance() };


	class ECSSystemManager
	{
	public:
		static ECSSystemManager& Instance()
		{
			static ECSSystemManager instance;
			return instance;
		}

		template <typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Systems.find(hashcode) == m_Systems.end()); //Make sure don't double register

			std::shared_ptr<T> system = std::make_shared<T>();
			m_Systems.insert({ hashcode, std::move(system) });
			//m_Systems.emplace(std::piecewise_construct, std::forward_as_tuple(hashcode), std::forward_as_tuple());
			return system;
		}

		template <typename T, typename... Arguments>
		std::shared_ptr<T> RegisterSystem(Arguments&&... arg)
		{
			auto hashcode = typeid(T).hash_code();

			assert(m_Systems.find(hashcode) == m_Systems.end()); //Make sure don't double register

			std::shared_ptr<T> system = std::make_shared<T>(std::forward<Arguments>(arg)...);
			m_Systems.insert({ hashcode, std::move(system) });
			return system;
		}

		template <typename T>
		void DeleteSystem()
		{
			auto hashcode = typeid(T).hash_code();
			if (m_Systems.find(hashcode) != m_Systems.end())
			{
				m_Systems.erase(hashcode);
			}
		}

		template <typename T>
		T* GetSystem()
		{
			auto hashcode = typeid(T).hash_code();
			if (m_Systems.find(hashcode) != m_Systems.end())
			{
				return dynamic_cast<T*>(m_Systems[hashcode].get());
			}

			//Can add assert here to tell user they getting a non registered system
			return nullptr;
		}

		void UpdateSystem()
		{
			for (auto& system : m_Systems)
			{
				system.second->Update();
			}
		}

		void OnDestroyGO()
		{
			for (auto& system : m_Systems)
			{
				system.second->OnDestroyGO();
			}
		}

		void ShutdownSystem()
		{
			for (auto& system : m_Systems)
			{
				system.second->Shutdown();
			}
		}

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		ECSSystemManager() {};
		ECSSystemManager(ECSSystemManager const&) = delete;
		void operator=(ECSSystemManager const&) = delete;
		void* operator new(size_t) = delete;

		std::map<size_t, std::shared_ptr<ECSSystem>> m_Systems;
	};
	static ECSSystemManager* _ecs_system_manager{ &ECSSystemManager::Instance() };
}