#include "pch.h"
#include "Prefab.h"
#include "Resource/Resource.h"
#include "Core/Logger.h"
#include "TREIncludes.h"
#include "Serialization.h"
#include "GameLoop.h"

namespace TRE
{
	void PrefabSystem::SavePrefabChild(Entity& child, bool newPrefab, std::string mainPrefabGUID)
	{
		bool validOverwrite{ !newPrefab && child->HasComponent<Prefabing>() }; // Doesnt need to be in existingPrefabs
		if (validOverwrite)
		{
			Prefabing& prefabExist{ child->GetComponent<Prefabing>() };

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
				TRE_CORE_WARN("[" + funcName + "] child (" + child->GetName() + ") is not an existing prefab instance! Creating a new prefab instead");
			}

			child->AddComponent<Prefabing>().m_PrefabGUID = MemoryManager::Instance().GenerateGUIDStr();
			child->GetComponent<Prefabing>().m_MainPrefabGUID = mainPrefabGUID;
		}

		// Recursively Save/Update child Prefabing
		for (Entity kid : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(child))
		{
			SavePrefabChild(kid, newPrefab, mainPrefabGUID);
		}
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

		// Create an Entity from ECSManager
		Entity instance{ ECSManager::Instance().CloneEntity(m_TempPrefab, m_TempPrefab->GetName()) };

		// Increment it's instance by one and add this in
		Prefabing& tempPrefab{ m_TempPrefab->GetComponent<Prefabing>() };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_PrefabGUID = tempPrefab.m_PrefabGUID;				// Assign it's m_PrefabGUID to m_PrefabGUID
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten

		Entity mainTempPrefab = m_TempPrefab;

		// Do the same for the children
		std::vector<std::string> children{ instance->GetComponent<Parenting>().m_Children };
		for (size_t i{}; i < children.size(); ++i) // Doing this instead cos string might be too long and cos errors :(
		{
			std::string childID{ children[i] };
			Entity child = CreatePrefabChild(childID, instance);
		}

		m_TempPrefab = mainTempPrefab;

		ResetTempPrefab();

		return instance;
	}

	void PrefabSystem::UpdateEntityInRegistry(Entity object, entt::registry& dstReg, std::string parentGUID, entt::entity parentEnt)
	{
		// Create prefab
		entt::entity ent = dstReg.create();

		// Clone each component of the object into the prefab
		for (auto [id, source_storage] : ECSManager::Instance().GetRegistry().storage())
		{
			auto destination_storage = dstReg.storage(id);
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

		// Force serialize new guid for the ent
		std::string prevGUID = dstReg.get<Properties>(ent).m_GUID;
		std::string entGUID = MemoryManager::Instance().GenerateGUIDStr();
		dstReg.get<Properties>(ent).m_GUID = entGUID;

		// Update it's parenting, if it have to update their parent guid
		if (parentGUID != "")
		{
			dstReg.get<Parenting>(ent).m_Parent = parentGUID;
			// Remove previous GUID string and add new id
			std::vector<std::string>& children{ dstReg.get<Parenting>(parentEnt).m_Children };
			auto it = std::find(children.begin(), children.end(), prevGUID);
			children.erase(it);
			children.emplace_back(entGUID);
		}

		// Clone it's children
		//for (Entity child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(object))
		for (std::string childGUID : object->GetComponent<Parenting>().m_Children)
		{
			UpdateEntityInRegistry(m_TempPrefabs[childGUID], dstReg, entGUID, ent);
		}
	}

	Entity PrefabSystem::CreatePrefabChild(std::string childGUID, Entity& parent)
	{
		// Create an Entity from ECSManager
		Entity childTemp{ m_TempPrefabs[childGUID] };
		Entity instance{ ECSManager::Instance().CloneEntity(childTemp, childTemp->GetName()) };

		// Increment it's instance by one and add this in
		Prefabing& tempPrefab{ childTemp->GetComponent<Prefabing>() };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_PrefabGUID = tempPrefab.m_PrefabGUID;				// Assign it's m_PrefabGUID to m_PrefabGUID
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten

		// Update parent and child relationship by assigning the correct guid
		std::vector<std::string>& parentsChildren{ parent->GetComponent<Parenting>().m_Children };
		auto it = std::find(parentsChildren.begin(), parentsChildren.end(), childTemp->GetGUID());
		parentsChildren.erase(it);
		parentsChildren.emplace_back(instance->GetGUID());
		instance->GetComponent<Parenting>().m_Parent = parent->GetGUID();
		instance->GetComponent<Transform>().UpdateLocalData(parent->GetComponent<Transform>());

		// Do the same for the children
		std::vector<std::string> children{ instance->GetComponent<Parenting>().m_Children };
		for (size_t i{}; i < children.size(); ++i) // Doing this instead cos string might be too long and cos errors :(
		{
			std::string childID{ children[i] };
			CreatePrefabChild(childID, instance);
		}

		return instance;
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
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") does not have Prefabing! Ignoring...");
			return false;
		}

		// Check if instance have same prefabGUID
		if (instance->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") m_PrefabGUID (" + instance->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")! Ignoring...");
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
					List.emplace_back(PropertyName, Data);

					(void)Flags;
				});
			instPropTable.emplace_back(instInspectableComp[i].first, List);
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

	void PrefabSystem::UnPrefabInstance(Entity instance)
	{
		// Auto return if it doesn't have prefabing
		if (!instance->HasComponent<Prefabing>()) return;

		for (std::string childGUID : instance->GetComponent<Parenting>().m_Children)
		{
			UnPrefabInstance(ECSManager::Instance().FindEntity(childGUID));
		}
		instance->RemoveComponent<Prefabing>();
	}

	void PrefabSystem::CreatePrefabAssetFile(std::string prefabGUID, std::string fileName, std::string filePath)
	{
		// Check if prefab exist based on latest directory
		DeserializePrefabDirectory();
		if (m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] prefabGUID (" + prefabGUID + ") does not exist in m_ExistingPrefabs! Ignoring command...");
			return;
		}

		// Try opening filePath
		std::ofstream file(filePath + fileName + FILESYS_PREFABASSTYPE);
		file << prefabGUID;
		file.close();
	}

	std::string PrefabSystem::ReadPrefabAssetFile(std::string filePathName)
	{
		// Try opening filePath
		std::ifstream file(filePathName);
		
		if (file.is_open() == false)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] filePathName (" + filePathName + ") does not exist! Returning empty string...");
			return "";
		}

		// Check if prefab exist based on latest directory
		DeserializePrefabDirectory();
		std::string GUID{};
		std::getline(file, GUID);
		file.close();
		if (m_ExistingPrefabs.find(GUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] GUID (" + GUID + ") does not exist in m_ExistingPrefabs! Returning empty string...");
			return "";
		}

		return GUID;
	}

	bool PrefabSystem::DeserializePrefabDirectory()
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
			std::ifstream file(prefabPath);
			
			if (!file.is_open())
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] PrefabDirectory GUID (" + prefabID + ") does not have a valid filepath (" + prefabPath + ")! Removing from m_ExistingPrefabs...");
				invalidGUIDs.emplace_back(prefabID);
				continue;
			}
			file.close();

			// Compare if the value inside the prefab matches same prefabID
			GetPrefabEntity(prefabPath);
			bool noneMatch{ true };
			for (auto tempPair : m_TempPrefabs)
			{
				if (tempPair.second->GetComponent<Prefabing>().m_PrefabGUID == prefabID)
				{
					noneMatch = false;
					break;
				}
			}

			if (noneMatch)
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] PrefabDirectory GUID (" + prefabID + ") does not match any of m_TempPrefabs GUIDs (" + std::to_string(m_TempPrefabs.size()) + ") in filepath (" + prefabPath + ")! Removing from m_ExistingPrefabs...");
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

		return !invalidGUIDs.empty();
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

	void PrefabSystem::UpdateAllInstances(std::string prefabGUID)
	{
		// Update all instances to match
		for (Entity ent : ECSManager::Instance().GetEntities<Prefabing>(true))
		{
			if (ent->GetComponent<Prefabing>().m_PrefabGUID == prefabGUID && UpdateInstance(ent, prefabGUID))
			{
				ent->GetComponent<Transform>().m_IsDirty = true;
			}
		}
	}

	bool PrefabSystem::UpdateInstance(Entity instance, std::string prefabGUID)
	{
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
			TRE_CORE_ERROR("[" + funcName + "] m_TempPrefab (" + m_TempPrefab->GetName() + ") m_PrefabGUID (" + m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")!");
			assert(m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID == prefabGUID);
		}

		// Check if instance belongs to the scene
		if (!ECSManager::Instance().IsValidEntity(instance))
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") does not belong to the current scene! Ignoring...");
			return false;
		}

		// Check if instance even have Prefabing to begin with
		if (!instance->HasComponent<Prefabing>())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") does not have Prefabing! Ignoring...");
			return false;
		}

		// Check if instance have same prefabGUID
		if (instance->GetComponent<Prefabing>().m_PrefabGUID != prefabGUID)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] instance (" + instance->GetName() + ") m_PrefabGUID (" + instance->GetComponent<Prefabing>().m_PrefabGUID + ") != prefabGUID (" + prefabGUID + ")! Ignoring...");
			return false;
		}

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
					List.emplace_back(PropertyName, Data);

					(void)Flags;
				});
			instPropTable.emplace_back(instInspectableComp[i].first, List);
		}

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
					List.emplace_back(PropertyName, Data);

					(void)Flags;
				});
			prefPropTable.emplace_back(prefInspectableComp[i].first, List);
		}

		// Add components marked as to be added into instance
		for (auto prefComp : prefInspectableComp)
		{
			// Get Prefab component name
			std::string compName{ prefComp.first };
			// Add comp if cannot find in instance
			if (std::find_if(instInspectableComp.begin(), instInspectableComp.end(), [&](std::pair<std::string, property::base*> p) { return p.first == compName; }) == instInspectableComp.end())
			{
				ECSManager::Instance().AddCompFromName(instance, compName);
				instInspectableComp = ECSManager::Instance().GetAllInspectableComponents(instance);
			}
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
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] Unable to find (" + prefPropTable[i].first + ") in instance when copying over prefab to instance");
			}
		}

		// Revert back to saved Values
		instance->GetComponent<Prefabing>() = instPrefabing;

		for (size_t i{}; i < instPropTable.size(); ++i)
		{
			// Only copy those that were registered as saved or is added
			bool isAddedComp{ instPrefabing.m_AddeddComps.find(instInspectableComp[i].first) != instPrefabing.m_AddeddComps.end() };
			auto it{ instPrefabing.m_Overrides.find(instInspectableComp[i].first) };
			if (isAddedComp)
			{
				property::base& compProp { *instInspectableComp[i].second };
				std::vector<property::entry> List{ instPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					property::set(compProp, Name.c_str(), Data);
				}
			}
			else if (it != instPrefabing.m_Overrides.end())
			{
				std::unordered_set<std::string> instCompData = it->second;

				property::base& compProp { *instInspectableComp[i].second };
				std::vector<property::entry> List{ instPropTable[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					if (instCompData.find(Name) != instCompData.end())
					{
						property::set(compProp, Name.c_str(), Data);
					}
				}
			}
		}
		instance->GetComponent<Properties>().m_Name = instName;
		instance->GetComponent<Properties>().m_GUID = instGUID;

		// Remove components marked as removed
		for (std::string compName : instPrefabing.m_RemovedComps)
		{
			ECSManager::Instance().RemCompFromName(instance, compName);
		}

		return true;
	}

	void PrefabSystem::ResetTempPrefab()
	{
		if (!m_TempPrefabs.empty())
		{
			for (auto entityPair : m_TempPrefabs)
			{
				// Clear the child and parent since they are all tempPrefabs
				entityPair.second->GetComponent<Parenting>().m_Parent = "";
				entityPair.second->GetComponent<Parenting>().m_Children.clear();
				MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(entityPair.second->m_Entity));
			}
			m_TempPrefabs.clear();
			m_TempPrefab = nullptr;
		}
	}
}