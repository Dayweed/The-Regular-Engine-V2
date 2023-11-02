#pragma once

#include "pch.h"
#include "ECS.h"
#include "System.h"

#define FILESYS_PREFABDEFFOLDER "../Assets/Prefabs/"			// Prefab Assets Default Folder
#define FILESYS_PREFABASSTYPE	".prefab"						// Prefab Assets Type		(bin/txt)
#define FILESYS_PREFABRSCFOLDER "../Resources/Prefabs/"			// Prefab Resource Default Folder
#define FILESYS_PREFABRSCTYPE	".json"							// Prefab Resource Type		(json)
#define FILESYS_PREFABDIR		"PrefabDirectory.json"			// Prefab Directory File
#define FILESYS_PREFABDIRNAME	"PrefabGUIDAndPrefabFilePath"	// Prefab Directory Name
#define FILESYS_PREFABDIRGUID	"m_ExistingPrefabsKey"			// Prefab Directory Key		(GUID)
#define FILESYS_PREFABDIRPATH	"m_ExistingPrefabsValue"		// Prefab Directory Value	(Path)

namespace TRE
{
	struct Prefabing
	{
		bool m_IsMainPrefab{};																// This is to ensure it only checks this with the filepath/update main prefab

		std::string m_MainPrefabGUID{};														// GUID of the main prefab (the main parent of the prefab)

		std::string m_PrefabGUID{};															// Prefabing GUID to be referred to when finding the correct doc / serializing
																							// This GUID exist as a reference to the object
																							// (NOT Properties::m_GUID which is used intenrally in the Engine)
																							// Blank if it is only an instance
		
		std::unordered_set<std::string> m_AddeddComps{};									// List of components removed

		std::unordered_set<std::string> m_RemovedComps{};									// List of components removed

		std::map<std::string, std::unordered_set<std::string>> m_Overrides{};				// List of components/properties added/overriten for a given component
																							// <Component Name, std::vector<Data Variable Name>>
																							// This will be updated for Components visible in INSPECTOR!
																							// This container will override any data in the instance after the prefab update the instance
																							// Any Component Name/Data Variable Name will be bold in Inspector
																							// Editting and changing back the values WILL still say it is overwritten
																							// Only way to remove is to revert everything based on prefab

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Prefabing, m_IsMainPrefab, m_MainPrefabGUID, m_PrefabGUID, m_RemovedComps, m_Overrides)
	};

	class PrefabOutputArchive
	{
	public:
		PrefabOutputArchive(std::string fileName, entt::registry& registry, int noOfEntities);
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

		int m_TotalEntities;

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

		// TO CHANGE
		void ConstructPhysicPrefab(Entity parent);

		void Init() override;
		void Update() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		// To "take over" the scene, returns prefab instance
		Entity DisplayPrefabInNewScene(std::string prefabGUID);
		void ReturnToScene();

		void CheckAndUpdateInstances();															// Goes through m_ExistingPrefabs and update all instances

		Entity GetDisplayedPrefab();

		bool IsValidPrefabResource(std::string prefabGUID);										// Checks if the string given is valid

		// (De)serializing Prefab
		std::string SavePrefabEntity(Entity object, bool newPrefab = true,						// Returns true if successful
										std::string assetPath = FILESYS_PREFABDEFFOLDER);
																								// Properties::m_GUID would not matter from now

		Entity CreatePrefabEntityInstance(std::string prefabGUID);								// Creates an Instance from the prefab

		std::unordered_map<std::string, Entity> GetPrefabEntity(std::string prefabFilePath);	// Assign m_TempPrefab to the prefab found in prefabFilePath and returns m_TempPrefabs, all entities found from path

		bool RevertInstance(Entity instance, std::string prefabGUID);							// Revert instance back to same data as prefab

		std::string ReadPrefabAssetFile(std::string filePathName);								// Returns GUID if file exist and GUID exist in prefab directory, else return empty string

	private:
		// Prefab Asset File
		// - Only contains string of PrefabGUID, will be used to direct type of prefab to spawn
		void CreatePrefabAssetFile(std::string prefabGUID, std::string fileName,				// Handles creating a Prefab Asset File based on the Resource File
								std::string filePath = FILESYS_PREFABDEFFOLDER);				

		Entity FindEntityBasedOnPrefabGUID(Entity object, std::string mainPrefabGUID);

		void SetUpRegistry(entt::registry& reg);

		std::string SerializePrefabOutputArchive(entt::registry& reg, std::string prefabGUID, std::string filePath, int NoOfEntities);	// Returns archive.filePath

		void SavePrefabChild(Entity& child, bool newPrefab, std::string mainPrefabGUID);

		void CreatePrefabChild(std::string childGUID, Entity& parent);							// Creates an Instance from the prefab (specifically for the kids! :D)

		void UpdateEntityInRegistry(Entity object, entt::registry& dstReg, std::string parentGUID = "", entt::entity parentEnt = {});	// Similar to SaveEntityInRegistry but for m_TempPrefabs

		// Deserializing list in prefabs directory into m_ExistingPrefabs
		bool DeserializePrefabDirectory();														// If a prefabFilePath no longer exist while checking:
																								// - All instances with Prefabing::m_Base == Prefabing::m_PrefabGUID will have their Prefabing Component removed
																								// - Remove Prefabing::m_PrefabGUID and prefabFilePath from the directory and reserialize immediately
																								// Returns true if it needs to serialize again

		void SerializePrefabDirectory();														// Auto serialize m_ExistingPrefabs into PrefabDirectory

		void UpdatePrefabDirectory(std::string prefabGUID, std::string prefabFilePath);			// Add prefabGUID and prefabFilePath into m_ExistingPrefabs and auto SerializeExistingPrefabs

		//void UpdateAllInstances(std::unordered_set<std::string>& instanceGUID, std::string prefabGUID);
		void UpdateAllInstances(std::string prefabGUID);

		// Returns true if instance is succesfully updates
		// Returns false if instance does not belong to prefabGUID or no longer exist
		bool UpdateInstance(Entity instance, std::string prefabGUID);

		void ResetTempPrefab();																	// Releases and Resets m_TempPrefab to nullptr 

		std::unordered_map<std::string, std::string> m_ExistingPrefabs;							// std::unordered_map<Prefabing::m_PrefabGUID, filePath>
																								// filePath: The entire string to access the file

		Entity m_DisplayedPrefab;																// This prefab is the one chosen when a .prefab is selected

		Entity m_TempPrefab;
		std::unordered_map<std::string, Entity> m_TempPrefabs;									// GUID found inside instance, Entity
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