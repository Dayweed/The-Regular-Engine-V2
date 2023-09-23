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

	std::string PrefabSystem::SavePrefabEntity(Entity object, bool newPrefab)
	{
		std::cout << "============= SavePrefabEntity " << object->GetName() << "|" << object->GetGUID() << "\n";
		std::string prefabGUID;
		std::string filePath;

		// Will use m_TempPrefab if it already is a prefab with a base in m_ExistingPrefabs
		bool validOverwrite{ !newPrefab && object->HasComponent<Prefabing>() && object->GetComponent<Prefabing>().m_PrefabGUID != "" && m_ExistingPrefabs.find(object->GetComponent<Prefabing>().m_PrefabGUID) != m_ExistingPrefabs.end() };
		if (validOverwrite)
		{
			std::cout << "Object: " << object->GetName() << "\n";
			Prefabing& prefabExist{ object->GetComponent<Prefabing>() };

			prefabGUID = prefabExist.m_PrefabGUID;
			filePath = m_ExistingPrefabs[prefabGUID];

			// Clear any overrides, this is the new prefab
			prefabExist.m_AddeddComps.clear();
			prefabExist.m_Overrides.clear();
			prefabExist.m_RemovedComps.clear();
		}
		else
		{
			if (!newPrefab)
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] object (" + object->GetName() + ") is not an existing prefab instance! Creating a new prefab instead");
			}

			prefabGUID = Resource::GetGUIDHex(Resource::GenerateGUID());
			filePath = "../Resources/Prefabs/" + object->GetName() + ".json";
			object->AddComponent<Prefabing>().m_PrefabGUID = prefabGUID;
			// Add as GUID
			object->GetComponent<Prefabing>().m_Instances.emplace(object->GetGUID());
		}

		entt::registry tmp;

		(void) tmp.view<
			Prefabing,
			Properties,
			Parenting,
			Transform,
			MeshRenderer,
			Camera,
			FEL,
			FAKEFEL
		>();

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

		// Specifically change entity properties m_GUID
		tmp.get<Properties>(ent).m_GUID = "";

		// Set up document
		PrefabOutputArchive arc(filePath, tmp);
		entt::snapshot snapshot{ tmp };
		// Serialize all entities and components
		snapshot.entities(arc)
			.component<Prefabing>(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
			;

		arc.Close();

		tmp.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		// Update all instances
		if (validOverwrite)
		{
			GetPrefabEntity(arc.GetFilePath());

			Prefabing& prefabExist{ m_TempPrefab->GetComponent<Prefabing>() };

			// Remove own self to ensure it wont get overwritten
			prefabExist.m_Instances.erase(object->GetGUID());

			// Update all instances to match
			UpdateAllInstances(prefabExist.m_Instances, prefabExist.m_PrefabGUID);
		}

		return prefabGUID;
	}

	Entity PrefabSystem::CreatePrefabEntityInstance(std::string prefabGUID)
	{
		std::cout << "============= CreatePrefabEntityInstance\n";

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

		// Create an Entity from ECSManager
		Entity instance{ ECSManager::Instance().CloneEntity(m_TempPrefab, m_TempPrefab->GetName()) };
		std::cout << "wihwnienwivnw " << m_TempPrefab->GetName() << "\n";
		std::cout << "eeeeeeeeeeeee " << instance->GetName() << "\n";

		// Increment it's instance by one and add this in
		m_TempPrefab->GetComponent<Prefabing>().m_Instances.emplace(instance->GetGUID());
		Prefabing& tempPrefab{ m_TempPrefab->GetComponent<Prefabing>() };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_PrefabGUID = tempPrefab.m_PrefabGUID;				// Assign it's m_PrefabGUID to m_PrefabGUID
		//instPrefab.m_Instances.clear();									// It is an instance, it does not have any instances
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten
		instPrefab.m_Instances = tempPrefab.m_Instances;				// Copy instances

		std::cout << "wihwnienwivnw " << m_TempPrefab->GetName() << "|" << m_TempPrefab->GetGUID() << "\n";
		std::cout << "eeeeeeeeeeeee " << instance->GetName() << "|" << instance->GetGUID() << "\n";
		std::cout << "ehiwhiow " << ECSManager::Instance().FindEntity(instance->GetGUID())->GetName() << "\n";
		// Reserialize tempPrefab
		bool updateSuccessful = UpdatePrefabEntity();
		std::cout << "eeeeeeeeeeeee " << ECSManager::Instance().FindEntity(instance->GetGUID())->GetName() << "\n";

		for (const std::string& instanceID : tempPrefab.m_Instances)
		{
			std::cout << "- " << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "|" << ECSManager::Instance().FindEntityID(ECSManager::Instance().FindEntity(instanceID)) << "\n";
		}

		if (!updateSuccessful)
		{
			ResetTempPrefab();
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] Failed to update PrefabEntity (" + m_TempPrefab->GetName() + ") succesfully");
			assert(updateSuccessful);
		}

		instance->GetComponent<Properties>().m_GUID = MemoryManager::Instance().GenerateGUIDStr();

		return instance;
	}

	Entity& PrefabSystem::GetPrefabEntity(std::string prefabFilePath)
	{
		entt::registry copy;
		PrefabInputArchive arc(prefabFilePath, copy);

		// REMEMBER TO UPDATE Prefab.cpp TOO!!!
		entt::basic_snapshot_loader loader(copy);
		loader.entities(arc)
			.component<Prefabing>(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
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

		std::cout << "============= Updating prefab Entity " << m_TempPrefab->GetName() << "\n";

		// Ensure Prefab exist
		Prefabing& prefabComp{ m_TempPrefab->GetComponent<Prefabing>() };
		std::string prefabGUID{ prefabComp.m_PrefabGUID };
		if (m_ExistingPrefabs.empty() || m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] prefabGUID (" + prefabGUID + ") is invalid!");
			assert(!m_ExistingPrefabs.empty());
			assert(m_ExistingPrefabs.find(prefabGUID) != m_ExistingPrefabs.end());
		}

		for (const std::string& instanceID : prefabComp.m_Instances)
		{
			std::cout << "- " << ECSManager::Instance().FindEntity(instanceID) << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "|" << ECSManager::Instance().FindEntityID(ECSManager::Instance().FindEntity(instanceID)) << "\n";
		}

		UpdateAllInstances(prefabComp.m_Instances, prefabGUID);

		std::string filePath{ m_ExistingPrefabs[prefabGUID] };

		entt::registry tmp;

		(void) tmp.view<
			Prefabing,
			Properties,
			Parenting,
			Transform,
			MeshRenderer,
			Camera,
			FEL,
			FAKEFEL
		>();

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
			.component<Prefabing>(arc)
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
		;

		arc.Close();

		tmp.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		// Release m_TempPrefab
		ResetTempPrefab();

		return true;
	}

	bool PrefabSystem::RevertInstance(Entity instance, std::string prefabGUID)
	{
		// Ensure it can check and copy over
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

		if (!m_TempPrefab)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] m_TempPrefab is still a nullptr!");
			assert(m_TempPrefab);
		}

		// Check if m_TempPrefab m_prefabGUID and prefabGUID is the same
		if (m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] m_TempPrefab m_PrefabGUID (" + m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")!");
			assert(m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID == prefabGUID);
		}

		// Check if instance even have Prefabing to begin with
		if (!instance->HasComponent<Prefabing>())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") does not have Prefabing! Removing from m_TempPrefab m_Instances...");
			return false;
		}

		// Check if instance have same prefabGUID
		if (instance->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") m_PrefabGUID (" + instance->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")! Removing from m_TempPrefab m_Instances...");
			return false;
		}

		// Copy over instance stuff
		// Save some values
		std::string instName{ instance->GetComponent<Properties>().m_Name };
		std::string instGUID{ instance->GetComponent<Properties>().m_GUID };
		Prefabing& pref{ instance->GetComponent<Prefabing>() };

		// Clear editted values
		pref.m_AddeddComps.clear();
		pref.m_Overrides.clear();
		pref.m_RemovedComps.clear();
		Prefabing instPrefabing{ pref };

		// Save properties list if there are stuff overwritten
		std::vector<std::pair<std::string, std::vector<property::entry>>> instPropTable;
		std::vector<std::pair<std::string, property::base*>> instInspectableComp = ECSManager::Instance().GetAllInspectableComponents(instance);
		for (size_t i{}; i < instInspectableComp.size(); ++i)
		{
			property::base& compProp { *instInspectableComp[i].second };
			std::vector<property::entry> List;
			property::SerializeEnum(compProp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
				{
					// If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
					assert(Flags.m_isScope == false || PropertyName.back() == ']');
					List.push_back(property::entry { PropertyName, Data });
				});
			instPropTable.push_back({ instInspectableComp[i].first, List });
		}

		// Remove all components in one entity
		for (auto&& elem : ECSManager::Instance().GetRegistry().storage()) {
			elem.second.remove(instance->m_Entity);
		}
		// Clone each component of the object into the clone
		for (auto&& curr : ECSManager::Instance().GetRegistry().storage())
		{
			if (auto& storage = curr.second; storage.contains(m_TempPrefab->m_Entity))
			{
				storage.emplace(instance->m_Entity, storage.get(m_TempPrefab->m_Entity));
			}
		}

		// Revert back to saved Values
		instance->GetComponent<Properties>().m_Name = instName;
		instance->GetComponent<Properties>().m_GUID = instGUID;
		instance->GetComponent<Prefabing>() = instPrefabing;

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
			if (tmpPrefabGUID != prefabID)
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

	void PrefabSystem::UpdateAllInstances(std::unordered_set<std::string>& instanceGUID, std::string prefabGUID)
	{
		std::cout << "===============INTO UpdateAllInstances\n";
		for (const std::string& instanceID : instanceGUID)
		{
			std::cout << "- " << ECSManager::Instance().FindEntity(instanceID) << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "|" << ECSManager::Instance().FindEntityID(ECSManager::Instance().FindEntity(instanceID)) << "\n";
		}

		// Update all instances to match
		std::vector<std::string> invalidInstance;
		for (const std::string& str : instanceGUID)
		{
			std::string instanceID{ str };
			std::cout << "\nTrying to update " << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "\n";
			if (!UpdateInstance(ECSManager::Instance().FindEntity(instanceID), prefabGUID))
			{
				invalidInstance.emplace_back(instanceID);
			}
			std::cout << "Finish Updating " << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "\n";

			std::cout << "NOW IS\n";
			for (const std::string& instanceID : instanceGUID)
			{
				std::cout << "- " << ECSManager::Instance().FindEntity(instanceID) << ECSManager::Instance().FindEntity(instanceID)->GetName() << "|" << instanceID << "|" << ECSManager::Instance().FindEntityID(ECSManager::Instance().FindEntity(instanceID)) << "\n";
			}
		}

		// Erase invalid instance from prefab
		for (std::string& instanceID : invalidInstance)
		{
			instanceGUID.erase(std::find(instanceGUID.begin(), instanceGUID.end(), instanceID));
		}
	}

	bool PrefabSystem::UpdateInstance(Entity instance, std::string prefabGUID)
	{
		std::cout << "============= Updating instance " << &instance << "|" << instance->GetName() << "|" << instance->GetGUID() << "\n";

		if (!m_TempPrefab)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] m_TempPrefab is a nullptr!");
			assert(m_TempPrefab);
		}

		// Check if m_TempPrefab m_prefabGUID and prefabGUID is the same
		if (m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] m_TempPrefab m_PrefabGUID (" + m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")!");
			assert(m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID == prefabGUID);
		}

		std::cout << "m_TempPrefab " << m_TempPrefab->GetName() << "|" << m_TempPrefab->GetGUID() << "\n";

		// Check if instance even have Prefabing to begin with
		if (!instance->HasComponent<Prefabing>())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") does not have Prefabing! Removing from m_TempPrefab m_Instances...");
			return false;
		}

		// Check if instance have same prefabGUID
		if (instance->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") m_PrefabGUID (" + instance->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")! Removing from m_TempPrefab m_Instances...");
			return false;
		}

		std::cout << "Original GUID: " << instance->GetComponent<Properties>().m_GUID << "|" << m_TempPrefab->GetComponent<Properties>().m_GUID << "\n";
		std::cout << "Original Entt: " << static_cast<ENTTID>(instance->m_Entity) << "|" << static_cast<ENTTID>(m_TempPrefab->m_Entity) << "\n";
		std::cout << "Original Properties: " << &instance->GetComponent<Properties>() << "|" << &m_TempPrefab->GetComponent<Properties>() << "\n";
		std::cout << "Original Transform: " << instance->GetComponent<Transform>().m_Position.x << "," << instance->GetComponent<Transform>().m_Position.y << "," << instance->GetComponent<Transform>().m_Position.z << "\n";

		// Copy over instance stuff
		// Save some values
		std::string instName{ instance->GetComponent<Properties>().m_Name };
		std::string instGUID{ instance->GetComponent<Properties>().m_GUID };
		Prefabing instPrefabing{ instance->GetComponent<Prefabing>() };

		// Save properties list if there are stuff overwritten
		std::vector<std::pair<std::string, std::vector<property::entry>>> instPropTable;
		std::vector<std::pair<std::string, property::base*>> instInspectableComp = ECSManager::Instance().GetAllInspectableComponents(instance);
		for (size_t i{}; i < instInspectableComp.size(); ++i)
		{
			property::base& compProp { *instInspectableComp[i].second };
			std::vector<property::entry> List;
			property::SerializeEnum(compProp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
				{
					// If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
					assert(Flags.m_isScope == false || PropertyName.back() == ']');
					List.push_back(property::entry { PropertyName, Data });
				});
			instPropTable.push_back({ instInspectableComp[i].first, List });
		}

		// Remove all components in one entity
		//for (auto&& elem : ECSManager::Instance().GetRegistry().storage()) {
		//	elem.second.remove(instance->m_Entity);
		//}
		////// Clone each component of the object into the clone
		////for (auto&& curr : ECSManager::Instance().GetRegistry().storage())
		////{
		////	if (auto& storage = curr.second; storage.contains(m_TempPrefab->m_Entity))
		////	{
		////		storage.emplace(instance->m_Entity, storage.get(m_TempPrefab->m_Entity));
		////	}
		////}

		// Copy tempPrefab Stuff
		std::vector<std::pair<std::string, std::vector<property::entry>>> prefPropTable;
		std::vector<std::pair<std::string, property::base*>> prefInspectableComp = ECSManager::Instance().GetAllInspectableComponents(m_TempPrefab);
		for (size_t i{}; i < prefInspectableComp.size(); ++i)
		{
			property::base& compProp { *prefInspectableComp[i].second };
			std::vector<property::entry> List;
			property::SerializeEnum(compProp, [&](std::string_view PropertyName, property::data&& Data, const property::table&, std::size_t, property::flags::type Flags)
				{
					// If we are dealing with a scope that is not an array someone may have change the SerializeEnum to a DisplayEnum they only show up there.
					assert(Flags.m_isScope == false || PropertyName.back() == ']');
					List.push_back(property::entry { PropertyName, Data });
				});
			prefPropTable.push_back({ prefInspectableComp[i].first, List });
		}

		// Copy stuff over to instance
		for (size_t i{}; i < prefPropTable.size(); ++i)
		{
			// Find component
			auto it{ std::find_if(instInspectableComp.begin(), instInspectableComp.end(), [&](std::pair<std::string, property::base*> p) { return p.first == prefPropTable[i].first; }) };
			if (it != instInspectableComp.end())
			{
				property::base& compProp { *it->second };
				std::vector<property::entry> List{ prefPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					property::set(compProp, Name.c_str(), Data);
				}
			}
			else
			{
				std::cout << "XXX Failed to find " << prefPropTable[i].first << "\n";
			}
		}

		//for (auto [id, source_storage] : ECSManager::Instance().GetRegistry().storage())
		//{
		//	auto destination_storage = ECSManager::Instance().GetRegistry().storage(id);
		//	if (destination_storage != nullptr && source_storage.contains(m_TempPrefab->m_Entity))
		//	{
		//		if (!destination_storage->contains(instance->m_Entity))
		//		{
		//			destination_storage->emplace(instance->m_Entity, source_storage.get(m_TempPrefab->m_Entity));
		//		}
		//		// Overwrite m_Entity if m_Entity already contains the component
		//		else
		//		{
		//			destination_storage->erase(instance->m_Entity);
		//			destination_storage->emplace(instance->m_Entity, source_storage.get(m_TempPrefab->m_Entity));
		//		}
		//	}
		//}

		std::cout << "Copied GUID: " << instance->GetComponent<Properties>().m_GUID << "|" << m_TempPrefab->GetComponent<Properties>().m_GUID << "\n";
		std::cout << "Copied Entt: " << static_cast<ENTTID>(instance->m_Entity) << "|" << static_cast<ENTTID>(m_TempPrefab->m_Entity) << "\n";
		std::cout << "Copied Properties: " << &instance->GetComponent<Properties>() << "|" << &m_TempPrefab->GetComponent<Properties>() << "\n";
		std::cout << "Copied Transform: " << instance->GetComponent<Transform>().m_Position.x << "," << instance->GetComponent<Transform>().m_Position.y << "," << instance->GetComponent<Transform>().m_Position.z << "\n";

		// Revert back to saved Values
		instance->GetComponent<Prefabing>() = instPrefabing;
		instance->GetComponent<Prefabing>().m_Instances = m_TempPrefab->GetComponent<Prefabing>().m_Instances;

		std::cout << instPrefabing.m_AddeddComps.size() << "|" << instPrefabing.m_Overrides.size() << "|" << instPrefabing.m_RemovedComps.size() << "\n";

		std::cout << "> m_Overrides\n";
		for (auto& vec : instPrefabing.m_Overrides)
		{
			std::cout << "- " << vec.first << "\n";
			for (std::string str : vec.second)
			{
				std::cout << "-- " << str << "\n";
			}
		}
		std::cout << "> m_AddeddComps\n";
		for (std::string str : instPrefabing.m_AddeddComps)
		{
			std::cout << "- " << str << "\n";
		}

		// Revert back those that are saved
		//std::string propCompName{ ComponentManager::Instance().GetComponentName<Properties>() };
		//for (size_t i{}; i < instPropTable.size(); ++i)
		//{
		//	property::base& compProp { *instInspectableComp[i].second };
		//	std::vector<property::entry> List{ instPropTable[i].second };
		//	for (const auto& [Name, Data] : List)
		//	{
		//		// Copy to compProp
		//		property::set(compProp, Name.c_str(), Data);
		//	}
		//}
		//instance->GetComponent<Properties>().m_Name = instName;
		//instance->GetComponent<Properties>().m_GUID = instGUID;
		for (size_t i{}; i < instPropTable.size(); ++i)
		{
			// Only copy those that were registered as saved or is added
			bool isProperties{ instInspectableComp[i].first == ComponentManager::Instance().GetComponentName<Properties>() };
			bool isAddedComp{ instPrefabing.m_AddeddComps.find(instInspectableComp[i].first) != instPrefabing.m_AddeddComps.end() };
			auto it{ instPrefabing.m_Overrides.find(instInspectableComp[i].first) };
			std::cout << "Finding " << instInspectableComp[i].first << "|" << isProperties << "|" << isAddedComp << "|" << (it != instPrefabing.m_Overrides.end()) << "\n";
			if (isProperties)
			{
				property::base& compProp { *instInspectableComp[i].second };
				std::vector<property::entry> List{ instPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					std::cout << "Finding Data " << Name << "\n";
					//std::cout << instance->GetName() << " Data " << std::get<std::string>(Data) << "...\n";
				}

				std::cout << "TESTNAME: " << instance->GetComponent<Properties>().m_Name << "|" << instName << "\n";
				std::cout << "TESTGUID: " << instance->GetComponent<Properties>().m_GUID << "|" << instGUID << "\n";
				std::cout << "TEMPGUID: " << m_TempPrefab->GetComponent<Properties>().m_GUID << "|" << ECSManager::Instance().FindEntity(m_TempPrefab->GetComponent<Properties>().m_GUID) << "\n";

				instance->GetComponent<Properties>().m_Name = instName;
				instance->GetComponent<Properties>().m_GUID = instGUID;

				std::cout << "Skip reverting properties\n";
			}
			else if (isAddedComp)
			{
				property::base& compProp { *instInspectableComp[i].second };
				std::vector<property::entry> List{ instPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					std::cout << "Finding Data " << Name << "| isAddComp \n";
					std::cout << instance->GetName() << " Moving Data " << Name << "...\n";
					property::set(compProp, Name.c_str(), Data);
					std::cout << instance->GetName() << " Moved Data " << Name << "!!!\n";
				}
			}
			else if (it != instPrefabing.m_Overrides.end())
			{
				std::unordered_set<std::string> instCompData = it->second;

				property::base& compProp { *instInspectableComp[i].second };
				std::cout << "Trying to assign to comp " << instInspectableComp[i].first << "\n";
				std::vector<property::entry> List{ instPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					std::cout << "Finding Data " << Name << "|" << isAddedComp << "|" << (instCompData.find(Name) != instCompData.end()) << "\n";
					if (instCompData.find(Name) != instCompData.end())
					{
						std::cout << instance->GetName() << " Moving Data " << Name << "...\n";
						property::set(compProp, Name.c_str(), Data);
						std::cout << instance->GetName() << " Moved Data " << Name << "!!!\n";
					}
				}
			}
		}

		std::cout << "Reverted GUID: " << instance->GetComponent<Properties>().m_GUID << "|" << m_TempPrefab->GetComponent<Properties>().m_GUID << "\n";
		std::cout << "Reverted Entt: " << static_cast<ENTTID>(instance->m_Entity) << "|" << static_cast<ENTTID>(m_TempPrefab->m_Entity) << "\n";
		std::cout << "Reverted Properties: " << &instance->GetComponent<Properties>() << "|" << &m_TempPrefab->GetComponent<Properties>() << "\n";
		std::cout << "Reverted Transform: " << instance->GetComponent<Transform>().m_Position.x << "," << instance->GetComponent<Transform>().m_Position.y << "," << instance->GetComponent<Transform>().m_Position.z << "\n";

		std::cout << "Done updating " << instance->GetName() << "|" << instance->GetGUID() << "\n";
		// Remove components marked as removed
		for (std::string compName : instPrefabing.m_RemovedComps)
		{
			ECSManager::Instance().RemCompFromName(instance, compName);
		}

		std::cout << "m_TempPrefab " << m_TempPrefab->GetName() << "|" << m_TempPrefab->GetGUID() << "\n";

		return true;
	}

	void PrefabSystem::ResetTempPrefab()
	{
		if (m_TempPrefab)
		{
			MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(m_TempPrefab->m_Entity));
			m_TempPrefab = nullptr;
		}
	}
}