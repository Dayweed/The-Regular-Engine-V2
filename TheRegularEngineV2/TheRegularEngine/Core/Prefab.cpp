#include "pch.h"
#include "Prefab.h"
#include "Resource/Resource.h"
#include "Core/Logger.h"
#include "TREIncludes.h"
#include "Serialization.h"

namespace TRE
{
	PrefabOutputArchive::PrefabOutputArchive(std::string fileName, entt::registry& registry) : m_FileName(fileName), m_Registry(registry)
	{
		m_Root = nlohmann::json::array();
	}

	void PrefabOutputArchive::operator()(entt::entity ent)
	{
		if (m_Registry.valid(ent))
		{
			m_Current.push_back(static_cast<uint32_t>(ent));
		}
	}

	void PrefabOutputArchive::operator()(std::underlying_type_t<entt::entity> u)
	{
		// First element of each array keeps the amount of elements. 
		if (m_Current.empty()) {
			m_Current = nlohmann::json::array();
			m_Current.push_back(1); // Saving only prefab
		}
		else
		{
			m_Root.push_back(m_Current);
			m_Current = nlohmann::json::array();
			m_Current.push_back(u);
		}
	}

	void PrefabOutputArchive::Close()
	{
		if (!m_Current.empty()) {
			m_Root.push_back(m_Current);
		}

		std::filesystem::path path{ m_FileName };
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << m_Root.dump(3);
		file.close();
	}

	std::string PrefabOutputArchive::AsString()
	{
		return m_Root.dump();
	}

	std::string PrefabOutputArchive::GetFilePath()
	{
		return m_FileName;
	}

	PrefabInputArchive::PrefabInputArchive(std::string fileName, entt::registry& registry) : m_FileName(fileName), m_Registry(registry)
	{
		std::ifstream file(m_FileName);
		m_Root = nlohmann::json::parse(file);
	}

	void PrefabInputArchive::operator()(entt::entity& ent)
	{
		uint32_t entID = m_Current[m_CurrentIdx].get<uint32_t>();
		ent = entt::entity(entID);
		m_CurrentIdx++;
	}

	void PrefabInputArchive::operator()(std::underlying_type_t<entt::entity>& u)
	{
		m_RootIdx++;
		if (m_RootIdx >= m_Root.size())
		{
			std::string funcName{ __FUNCTION__ };
			std::string error{ "[" + funcName + "] InputArchive have m_RootIdx " + std::to_string(m_RootIdx) + " < " + std::to_string(m_Root.size()) };
			TRE_CORE_ERROR(error);
			assert(m_RootIdx < m_Root.size());
			return;
		}
		m_Current = m_Root[m_RootIdx];
		m_CurrentIdx = 0;

		int size = m_Current[0].get<int>();
		m_CurrentIdx++;
		// Pass amount to entt
		u = static_cast<std::underlying_type_t<entt::entity>>(size);
	}

	void PrefabSystem::Update()
	{

	}

	void PrefabSystem::OnDestroyGO()
	{

	}

	void PrefabSystem::Shutdown()
	{

	}

	bool PrefabSystem::SavePrefabEntity(Entity object)
	{
		std::string prefabGUID{ Resource::GetGUIDHex(Resource::GenerateGUID()) };

		std::string filePath{ "../Resources/Prefabs/" + object->GetName() + ".prefab"};

		std::string funcName{ __FUNCTION__ };
		TRE_INFO("[" + funcName + "] Serializing to " + filePath);

		entt::registry tmp;

		(void) tmp.view<
			Properties,
			Parenting,
			Transform,
			MeshRenderer,
			Camera,
			FEL>();

		// Create prefab
		entt::entity ent = tmp.create();

		// Clone each component of the object into the prefab
		for (auto [id, source_storage] : ECSManager::Instance().GetRegistry().storage())
		{
			auto destination_storage = tmp.storage(id);
			if (destination_storage != nullptr && source_storage.contains(object->m_Entity))
			{
				if (!destination_storage->contains(ent))
				{
					destination_storage->emplace(ent, source_storage.get(object->m_Entity));
				}
				// Overwrite m_Entity if m_Entity already contains the component
				else
				{
					destination_storage->erase(ent);
					destination_storage->emplace(ent, source_storage.get(object->m_Entity));
				}
			}
		}

		// Set up document
		PrefabOutputArchive arc(filePath, tmp);
		entt::snapshot snapshot{ tmp };
		// Serialize all entities and components
		snapshot.entities(arc)
			//.component<Prefabing>(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc);
			;

		arc.Close();

		tmp.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		return true;
	}

	Entity PrefabSystem::CreatePrefabEntityInstance(std::string prefabGUID)
	{
		// Find Entity filePath from prefabGUID
		DeserializePrefabDirectory();	// Ensure it is updated

		if (m_ExistingPrefabs.empty() || m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] prefabGUID (" + prefabGUID + ") is invalid!");
			assert(!m_ExistingPrefabs.empty());
			assert(m_ExistingPrefabs.find(prefabGUID) != m_ExistingPrefabs.end());
		}
		std::string prefabFilePath{ m_ExistingPrefabs[prefabGUID] };

		// Get Enitity based on filePath
		GetPrefabEntity(prefabFilePath); // m_TempPrefab is already assign in it

		// Get size
		Prefabing& tempPrefab{ m_TempPrefab->GetComponent<Prefabing>() };
		size_t instSize{ tempPrefab.m_Instances.size() + 1 };

		// Create an Entity from ECSManager
		Entity instance{ ECSManager::Instance().CreateEntity(m_TempPrefab->GetName() + " (" + std::to_string(instSize) + ")") };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_Base = tempPrefab.m_PrefabGUID;					// Make base to m_PrefabGUID
		instPrefab.m_PrefabGUID = "";									// It is a newborn, it is not an actual prefab
		instPrefab.m_Instances.clear();									// It is a newborn, it does not have any instances
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten

		// Increment it's instance by one and add this in
		tempPrefab.m_Instances.emplace_back(instance->GetGUID());

		// Reserialize tempPrefab
		bool updateSuccessful = UpdatePrefabEntity();
		if (!updateSuccessful)
		{
			ResetTempPrefab();
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] Failed to update PrefabEntity (" + m_TempPrefab->GetName() + ") succesfully");
			assert(updateSuccessful);
		}

		return instance;
	}

	Entity& PrefabSystem::GetPrefabEntity(std::string prefabFilePath)
	{
		entt::registry copy;
		PrefabInputArchive arc(prefabFilePath, copy);

		// REMEMBER TO UPDATE Prefab.cpp TOO!!!
		entt::basic_snapshot_loader loader(copy);
		loader.entities(arc)
			//.component<Prefabing>(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc)
			;

		// Clone each component of the object into the prefab
		// USES MEMORYMANAGER INSTEAD OF ECSMANAGER, ECSManager should never know this exist!
		m_TempPrefab = MemoryManager::Instance().GetUndeployedEntity();
		unsigned int count{};
		copy.each([&](entt::entity srcEntity) {
			++count;	// Add count
			for (auto [id, source_storage] : copy.storage())
			{
				auto destination_storage = ECSManager::Instance().GetRegistry().storage(id);
				if (destination_storage != nullptr && source_storage.contains(srcEntity))
				{
					if (!destination_storage->contains(m_TempPrefab->m_Entity))
					{
						destination_storage->emplace(m_TempPrefab->m_Entity, source_storage.get(srcEntity));
					}
					// Overwrite m_Entity if m_Entity already contains the component
					else
					{
						destination_storage->erase(m_TempPrefab->m_Entity);
						destination_storage->emplace(m_TempPrefab->m_Entity, source_storage.get(srcEntity));
					}
				}
			}
		});

		// Throw error if copy registry contained multiple instance
		if (count != 1)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] count is (" + std::to_string(count) + ") != 1!");
			ResetTempPrefab();
			assert(count == 1);
		}

		// Throw error if m_TempPrefab does not even have Prefabing
		if (!m_TempPrefab->HasComponent<Prefabing>())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + m_TempPrefab->GetName() + " does not even have Component Prefabing! Therefore it is invalid!");
			ResetTempPrefab();
			assert(m_TempPrefab->HasComponent<Prefabing>());
		}

		return m_TempPrefab;
	}

	bool PrefabSystem::UpdatePrefabEntity()
	{
		if (!m_TempPrefab)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] m_TempPrefab is a nullptr!");
			return false;
		}

		std::string prefabGUID{ m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID };
		if (m_ExistingPrefabs.empty() || m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] prefabGUID (" + prefabGUID + ") is invalid!");
			assert(!m_ExistingPrefabs.empty());
			assert(m_ExistingPrefabs.find(prefabGUID) != m_ExistingPrefabs.end());
		}

		std::string filePath{ m_ExistingPrefabs[prefabGUID] };

		std::string funcName{ __FUNCTION__ };
		TRE_INFO("[" + funcName + "] Serializing to " + filePath);

		entt::registry tmp;

		(void) tmp.view<
			Properties,
			Parenting,
			Transform,
			MeshRenderer,
			Camera,
			FEL>();

		// Create prefab
		entt::entity ent = tmp.create();

		// Clone each component of the object into the prefab
		for (auto [id, source_storage] : ECSManager::Instance().GetRegistry().storage())
		{
			auto destination_storage = tmp.storage(id);
			if (destination_storage != nullptr && source_storage.contains(m_TempPrefab->m_Entity))
			{
				if (!destination_storage->contains(ent))
				{
					destination_storage->emplace(ent, source_storage.get(m_TempPrefab->m_Entity));
				}
				// Overwrite m_Entity if m_Entity already contains the component
				else
				{
					destination_storage->erase(ent);
					destination_storage->emplace(ent, source_storage.get(m_TempPrefab->m_Entity));
				}
			}
		}

		// Set up document
		PrefabOutputArchive arc(filePath, tmp);
		entt::snapshot snapshot{ tmp };
		// Serialize all entities and components
		snapshot.entities(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc);
		;

		arc.Close();

		tmp.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		// Release m_TempPrefab
		ResetTempPrefab();

		return true;
	}

	void PrefabSystem::DeserializePrefabDirectory()
	{
		// Prepare file name [For deserialization too]
		std::string filePathString{ FILESYS_PREFABDIR };

		// Start unloading file properties
		ObjectDeserializer deserializerProperties;
		deserializerProperties.loadFile(filePathString.c_str());

		std::vector<std::string> m_ExistingPrefabsFirst;
		std::vector<std::string> m_ExistingPrefabsSecond;
		deserializerProperties.get_value(FILESYS_PREFABDIRNAME, FILESYS_PREFABDIRGUID, m_ExistingPrefabsFirst);
		deserializerProperties.get_value(FILESYS_PREFABDIRNAME, FILESYS_PREFABDIRPATH, m_ExistingPrefabsSecond);

		// Ensure both vectors are same size
		if (m_ExistingPrefabsFirst.size() != m_ExistingPrefabsSecond.size())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] PrefabDirectory Deserializing does not have same size!");
			assert(m_ExistingPrefabsFirst.size() == m_ExistingPrefabsSecond.size());
		}

		// Update m_ExistingPrefabs
		m_ExistingPrefabs.clear();
		for (size_t i{}; i < m_ExistingPrefabsFirst.size(); ++i)
		{
			m_ExistingPrefabs.emplace(std::piecewise_construct, std::forward_as_tuple(m_ExistingPrefabsFirst[i]), std::forward_as_tuple(m_ExistingPrefabsSecond[i]));
		}

		// Ensure all Prefabs are valid
		std::vector<std::string> invalidGUIDs;
		for (std::pair<const std::string, std::string>& prefab : m_ExistingPrefabs)
		{
			std::string prefabID{ prefab.first };
			std::string prefabPath{ prefab.second };

			// Try opening filePath
			std::ifstream file;
			file.open(prefabPath);
			if (!file)
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] PrefabDirectory GUID (" + prefabID + ") does not have a valid filepath (" + prefabPath + ")! Removing from m_ExistingPrefabs...");
				invalidGUIDs.emplace_back(prefabID);
				continue;
			}
			file.close();

			// Compare if the value inside the prefab matches same prefabID
			GetPrefabEntity(prefabPath);
			std::string tmpPrefabGUID{ GetPrefabEntity(prefabPath)->GetComponent<Prefabing>().m_PrefabGUID };
			if (tmpPrefabGUID == prefabID)
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] PrefabDirectory GUID (" + prefabID + ") does not match m_PrefabGUID (" + tmpPrefabGUID + ") in filepath (" + prefabPath + ")! Removing from m_ExistingPrefabs...");
				invalidGUIDs.emplace_back(prefabID);
				ResetTempPrefab();
				continue;
			}

			// Reset it
			ResetTempPrefab();
		}

		// Delete invalid GUIDs
		for (std::string& invalid : invalidGUIDs)
		{
			m_ExistingPrefabs.erase(invalid);
		}
	}

	void PrefabSystem::SerializePrefabDirectory()
	{
		// Set up document
		rapidjson::Document fileSystemDoc;
		std::string filePathString{ FILESYS_PREFABDIR };
		fileSystemDoc.SetObject();
		WriteToExternalFile(fileSystemDoc, filePathString.c_str());

		ObjectSerializer fileSerial(filePathString.c_str());
		Allocator allocator = fileSerial.getDoc().GetAllocator();

		// Folder Name
		ObjectBuilder folder;

		// Seperate maps to different parts for rapidjson
		std::vector<std::string> m_ExistingPrefabsFirst;
		std::vector<std::string> m_ExistingPrefabsSecond;
		// std::unordered_map<Prefabing::m_PrefabGUID, filePath>
		for (std::pair<const std::string, std::string>& prefab : m_ExistingPrefabs)
		{
			m_ExistingPrefabsFirst.emplace_back(prefab.first);
			m_ExistingPrefabsSecond.emplace_back(prefab.second);
		}

		folder.insertArray(FILESYS_PREFABDIRGUID, m_ExistingPrefabsFirst, allocator);
		folder.insertArray(FILESYS_PREFABDIRPATH, m_ExistingPrefabsSecond, allocator);

		fileSerial.AddObjectToDoc(folder.getValue(), FILESYS_PREFABDIRNAME);

		// Write to Doc
		fileSerial.writeToDoc(filePathString.c_str());

		std::string funcName{ __FUNCTION__ };
		TRE_CORE_INFO("[" + funcName + "] Updating Prefab Directory at " + filePathString);
	}

	void PrefabSystem::UpdatePrefabDirectory(std::string prefabGUID, std::string prefabFilePath)
	{
		DeserializePrefabDirectory();	// Ensure it is updated

		// Overwriting existing prefabGUID
		if (!m_ExistingPrefabs.empty() && m_ExistingPrefabs.find(prefabGUID) != m_ExistingPrefabs.end())
		{
			m_ExistingPrefabs[prefabGUID] = prefabFilePath;
		}
		// Create New
		else
		{
			m_ExistingPrefabs.emplace(std::piecewise_construct, std::forward_as_tuple(prefabGUID), std::forward_as_tuple(prefabFilePath));
		}

		SerializePrefabDirectory();
	}

	bool PrefabSystem::UpdateInstance(Entity instance, std::string prefabGUID)
	{

		return true;
	}

	void PrefabSystem::ResetTempPrefab()
	{
		MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(m_TempPrefab->m_Entity));
		m_TempPrefab = nullptr;
	}
}