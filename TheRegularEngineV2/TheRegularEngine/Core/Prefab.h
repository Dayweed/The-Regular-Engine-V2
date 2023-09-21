#pragma once

#include "pch.h"
#include "ECS.h"

namespace TRE
{
	struct Prefabing
	{
		std::string m_Base{};																// ID of file name (.json)
		std::vector<std::string> m_Instances{};												// Instances that are based on this Entity (ID of file name (.json))
		// std::pair<Component Name, std::vector<Data Var Name>>
		std::vector<std::pair<std::string, std::vector<std::string>>> m_Overrides{};		// List of components/properties added/removed/overriten for a given component
	};

	class PrefabOutputArchive
	{
	public:
		PrefabOutputArchive(std::string fileName, entt::registry& registry);
		void operator()(entt::entity ent);
		void operator()(std::underlying_type_t<entt::entity> u);
		template <typename T>
		void operator()(entt::entity ent, const T& t);
		void Close();
		std::string AsString();
		std::string GetFilePath();

	private:
		std::string m_FileName;
		entt::registry& m_Registry;

		nlohmann::json m_Root;
		nlohmann::json m_Current;
	};

	class PrefabInputArchive
	{
	public:
		PrefabInputArchive(std::string fileName, entt::registry& registry);
		void operator()(entt::entity& ent);
		void operator()(std::underlying_type_t<entt::entity>& u);
		template <typename T>
		void operator()(entt::entity& ent, T& t);
	private:
		std::string m_FileName;
		entt::registry& m_Registry;

		nlohmann::json m_Root;
		nlohmann::json m_Current;

		int m_RootIdx = -1;
		int m_CurrentIdx = 0;
	};

	class PrefabManager : public ECSSystem
	{
	public:
		PrefabManager() = default;
		~PrefabManager() = default;

		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		std::string SavePrefabEntity(Entity object);
	};


	template <typename T>
	void PrefabOutputArchive::operator()(entt::entity ent, const T& t)
	{
		if (m_Registry.valid(ent))
		{
			m_Current.push_back(static_cast<uint32_t>(ent)); // persist the entity id of the following component

			nlohmann::json json = t;
			m_Current.push_back(json);
		}
	}

	template <typename T>
	void PrefabInputArchive::operator()(entt::entity& ent, T& t)
	{
		nlohmann::json componentData = m_Current[m_CurrentIdx * 2];

		if (!componentData.is_null() && (m_CurrentIdx * 2 - 1) < m_Current.size())
		{
			auto comp = componentData.get<T>();
			t = comp;

			// Last element is the entID
			uint32_t entID = m_Current[m_CurrentIdx * 2 - 1];
			ent = entt::entity(entID);
			m_CurrentIdx++;
		}
		else if (componentData.is_null())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is NULL");
			assert(!componentData.is_null());
		}
		else
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " have an index [" + std::to_string(m_CurrentIdx * 2 - 1) + "] that is >= container size of [" + std::to_string(m_Current.size()) + "]\n");
			assert((m_CurrentIdx * 2 - 1) < m_Current.size());
		}
	}
}