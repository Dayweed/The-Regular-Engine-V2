#pragma once

#include "pch.h"
#include "ECS.h"
#include "System.h"

#define FILESYS_PREFABDIR		"PrefabDirectory.json"
#define FILESYS_PREFABDIRNAME	"PrefabGUIDAndPrefabFilePath"
#define FILESYS_PREFABDIRGUID	"m_ExistingPrefabsKey"
#define FILESYS_PREFABDIRPATH	"m_ExistingPrefabsValue"

namespace TRE
{
	struct Prefabing
	{
		std::string m_PrefabGUID{};															// Prefabing GUID to be referred to when finding the correct doc / serializing
																							// This GUID exist as a reference to the object
																							// (NOT Properties::m_GUID which is used intenrally in the Engine)
																							// Blank if it is only an instance

		//std::string m_BasedGUID{};															// m_PrefabGUID of the prefab it is finding from

		std::vector<std::string> m_Instances{};												// Instances that are based on this Entity (Properties::m_GUID to easily get them)
																							// This gets updated everytime an Instance is created
																							// Automatically updates all instances if the Prefab is saved
																							// Skips and removes instances when saving if:
																							// - Instance no longer exist
																							// - Instance does not have Prefabing Component
																							// - m_Base of instance does not match the m_PrefabGUID
		
		std::vector<std::string> m_AddeddComps{};											// List of components removed

		std::vector<std::string> m_RemovedComps{};											// List of components removed

		std::map<std::string, std::vector<std::string>> m_Overrides{};						// List of components/properties added/overriten for a given component
																							// <Component Name, std::vector<Data Variable Name>>
																							// This will be updated for Components visible in INSPECTOR!
																							// This container will override any data in the instance after the prefab update the instance
																							// Any Component Name/Data Variable Name will be bold in Inspector
																							// Editting and changing back the values WILL still say it is overwritten
																							// Only way to remove is to revert everything based on prefab

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Prefabing, m_PrefabGUID, m_Instances, m_RemovedComps, m_Overrides)
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

	class PrefabSystem : public ECSSystem
	{
	public:
		PrefabSystem() = default;
		~PrefabSystem() = default;

		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		// (De)serializing Prefab
		std::string SavePrefabEntity(Entity object, bool newPrefab = true);						// Returns true if successful
																								// Properties::m_GUID would not matter from now

		Entity CreatePrefabEntityInstance(std::string prefabGUID);								// Creates an Instance from the prefab

		Entity& GetPrefabEntity(std::string prefabFilePath);									// Assign m_TempPrefab to the prefab found in prefabFilePath and returns m_TempPrefab

		bool UpdatePrefabEntity();																// For Overwriting existing m_PrefabGUID! No param to force use m_TempPrefab
																								// Uses m_TempPrefab to update in Inspector, does nothing if it is nullptr
																								// Returns true if m_TempPrefab exist and was successfully updated

		bool RevertInstance(Entity instance, std::string prefabGUID);							// Revert instance back to same data as prefab

	private:
		// Deserializing list in prefabs directory into m_ExistingPrefabs
		void DeserializePrefabDirectory();														// If a prefabFilePath no longer exist while checking:
																								// - All instances with Prefabing::m_Base == Prefabing::m_PrefabGUID will have their Prefabing Component removed
																								// - Remove Prefabing::m_PrefabGUID and prefabFilePath from the directory and reserialize immediately

		void SerializePrefabDirectory();														// Auto serialize m_ExistingPrefabs into PrefabDirectory

		void UpdatePrefabDirectory(std::string prefabGUID, std::string prefabFilePath);			// Add prefabGUID and prefabFilePath into m_ExistingPrefabs and auto SerializeExistingPrefabs

		// Returns true if instance is succesfully updates
		// Returns false if instance does not belong to prefabGUID or no longer exist
		bool UpdateInstance(Entity instance, std::string prefabGUID);

		void ResetTempPrefab();																	// Releases and Resets m_TempPrefab to nullptr 

		std::unordered_map<std::string, std::string> m_ExistingPrefabs;							// std::unordered_map<Prefabing::m_PrefabGUID, filePath>
																								// filePath: The entire string to access the file

		Entity m_TempPrefab;
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