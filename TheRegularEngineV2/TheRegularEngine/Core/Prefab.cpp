#include "pch.h"
#include "Prefab.h"
#include "Resource/Resource.h"
#include "Core/Logger.h"
#include "TREIncludes.h"
#include "Serialization.h"
#include "GameLoop.h"

namespace TRE
{
	PrefabOutputArchive::PrefabOutputArchive(std::string fileName, entt::registry& registry, int noOfEntities) : m_FileName(fileName), m_Registry(registry), m_TotalEntities(noOfEntities)
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
			m_Current.push_back(m_TotalEntities); // Saving only prefab
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

	void PrefabSystem::Init()
	{
		DeserializePrefabDirectory();
		SerializePrefabDirectory();
	}

	void PrefabSystem::Update()
	{

	}

	void PrefabSystem::OnReset()
	{

	}

	void PrefabSystem::OnDestroyEntities()
	{

	}

	void PrefabSystem::Shutdown()
	{

	}

	Entity PrefabSystem::DisplayPrefabInNewScene(std::string prefabGUID)
	{
		// Store the scene if it wasn't displaying a prefab
		if (!GameLoop::Instance().GetDisplayingPrefab())
		{
			// Destroys all undeployed entities
			MemoryManager::Instance().ClearUndeployed();

			// Save the registry
			GameLoop::Instance().GetBackUpRegistry().clear();
			ECSManager::Instance().SaveRegistry(GameLoop::Instance().GetBackUpRegistry());
		}

		// Clear the "scene" and show the displayed prefab
		ECSManager::Instance().DestroyAll();

		// Create Mandotary Main Camera
		Entity MainCamera = ECSManager::Instance().CreateEntity("Main Camera");
		MainCamera->AddComponent<Camera>();
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(MainCamera, true);

		m_DisplayedPrefab = ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(prefabGUID);

		GameLoop::Instance().SetDisplayingPrefab(true);

		return m_DisplayedPrefab;
	}

	void PrefabSystem::ReturnToScene()
	{
		// Copy registry and components
		ECSManager::Instance().CopyRegistry(GameLoop::Instance().GetBackUpRegistry());
		// Clear Backup
		GameLoop::Instance().GetBackUpRegistry().clear();
		// Update all Prefabs
		CheckAndUpdateInstances();
		// Auto set back to false
		GameLoop::Instance().SetDisplayingPrefab(false);
	}

	void PrefabSystem::CheckAndUpdateInstances()
	{
		DeserializePrefabDirectory();

		auto rscPaths{ m_ExistingPrefabs };
		for (auto rscPath : m_ExistingPrefabs)
		{
			GetPrefabEntity(rscPath.second );

			for (auto prefabPair : m_TempPrefabs)
			{
				m_TempPrefab = prefabPair.second;
				Prefabing& prefabComp{ m_TempPrefab->GetComponent<Prefabing>() };
				std::string prefabGUID{ prefabComp.m_PrefabGUID };

				UpdateAllInstances(prefabComp.m_Instances, prefabGUID);
			}

			ResetTempPrefab();
		}
	}

	Entity PrefabSystem::GetDisplayedPrefab()
	{
		return m_DisplayedPrefab;
	}

	std::string PrefabSystem::SavePrefabEntity(Entity object, bool newPrefab, std::string assetPath)
	{
		std::string prefabGUID;
		std::string filePath;

		// Ensure this object main prefab is not another prefab!
		if (object->HasComponent<Prefabing>())
		{
			Prefabing& prefabComp{ object->GetComponent<Prefabing>() };
			if (prefabComp.m_MainPrefabGUID != prefabComp.m_PrefabGUID)
			{
				// Overwrite object with the main object instead
				Entity MainObject = FindEntityBasedOnPrefabGUID(object, prefabComp.m_MainPrefabGUID);
				if (MainObject)
				{
					object = MainObject;
				}
				else
				{
					std::string funcName{ __FUNCTION__ };
					TRE_ERROR("[" + funcName + "] Unable to find Main Entity from (" + object->GetName() + ")!");
					assert(MainObject);
				}
			}
		}

		bool validOverwrite{ !newPrefab && object->HasComponent<Prefabing>() && m_ExistingPrefabs.find(object->GetComponent<Prefabing>().m_PrefabGUID) != m_ExistingPrefabs.end() };
		if (validOverwrite)
		{
			Prefabing& prefabExist{ object->GetComponent<Prefabing>() };

			prefabGUID = prefabExist.m_PrefabGUID;
			filePath = m_ExistingPrefabs[prefabGUID];

			// Clear any overrides, this is the new prefab
			prefabExist.m_AddeddComps.clear();
			prefabExist.m_Overrides.clear();
			prefabExist.m_RemovedComps.clear();

			prefabExist.m_IsMainPrefab = true;
		}
		// New Prefab
		else
		{
			if (!newPrefab)
			{
				std::string funcName{ __FUNCTION__ };
				TRE_CORE_WARN("[" + funcName + "] object (" + object->GetName() + ") is not an existing prefab instance! Creating a new prefab instead");
			}

			prefabGUID = Resource::GetGUIDHex(Resource::GenerateGUID());
			filePath = FILESYS_PREFABRSCFOLDER + object->GetName() + FILESYS_PREFABRSCTYPE;
			object->AddComponent<Prefabing>().m_PrefabGUID = prefabGUID;
			object->GetComponent<Prefabing>().m_MainPrefabGUID = prefabGUID;
			// Add as GUID
			object->GetComponent<Prefabing>().m_Instances.emplace(object->GetGUID());

			object->GetComponent<Prefabing>().m_IsMainPrefab = true;
		}

		// Save/Update child Prefabing
		for (Entity child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(object))
		{
			SavePrefabChild(child, newPrefab, prefabGUID);
		}

		// SERIALIZING PREFAB
		//
		entt::registry tmp;
		SetUpRegistry(tmp);

		// Clone Prefab
		SaveEntityInRegistry(object, tmp);

		// GetNoOfEntities
		int noOfEntities{};
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetTotalEntities(noOfEntities, object);

		std::string arcFilePath = SerializePrefabOutputArchive(tmp, prefabGUID, filePath, noOfEntities);

		// Create Prefab Asset File for accessing
		CreatePrefabAssetFile(prefabGUID, object->GetName(), assetPath);


		// Update all instances
		if (validOverwrite)
		{
			GetPrefabEntity(arcFilePath);

			for (auto prefabPair : m_TempPrefabs)
			{
				m_TempPrefab = prefabPair.second;
				Prefabing& prefabComp{ m_TempPrefab->GetComponent<Prefabing>() };
				std::string prefabGUID{ prefabComp.m_PrefabGUID };

				UpdateAllInstances(prefabComp.m_Instances, prefabGUID);
			}

			ResetTempPrefab();
		}

		return prefabGUID;
	}

	Entity PrefabSystem::FindEntityBasedOnPrefabGUID(Entity object, std::string mainPrefabGUID)
	{
		if (object && object->HasComponent<Prefabing>())
		{
			Prefabing& prefabComp{ object->GetComponent<Prefabing>() };
			if (prefabComp.m_MainPrefabGUID == prefabComp.m_PrefabGUID)
			{
				return object;
			}

			return FindEntityBasedOnPrefabGUID(ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(object), mainPrefabGUID);
		}

		if (!object)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] object is a nullptr! Returning nullptr");
		}
		else
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] object (" + object->GetName() + ") is not an existing prefab instance! Returning nullptr");
		}

		return nullptr;
	}

	void PrefabSystem::SetUpRegistry(entt::registry& reg)
	{
		(void)reg.view<
			Prefabing,
			Parenting,
			Properties,
			Transform,
			MeshRenderer,
			Camera,
			SphereCollider,
			BoxCollider,
			Rigidbody,
			FEL,
			FAKEFEL
		>();
	}

	std::string PrefabSystem::SerializePrefabOutputArchive(entt::registry& reg, std::string prefabGUID, std::string filePath, int NoOfEntities)
	{
		// Set up document
		PrefabOutputArchive arc(filePath, reg, NoOfEntities);
		entt::snapshot snapshot{ reg };
		// Serialize all entities and components
		snapshot.entities(arc)
			.component<Prefabing>(arc)
			.component<Parenting>(arc)
			.component<Properties>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<Rigidbody>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
			;

		arc.Close();

		reg.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		return arc.GetFilePath();
	}

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

			child->AddComponent<Prefabing>().m_PrefabGUID = Resource::GetGUIDHex(Resource::GenerateGUID());
			child->GetComponent<Prefabing>().m_MainPrefabGUID = mainPrefabGUID;
			// Add as GUID
			child->GetComponent<Prefabing>().m_Instances.emplace(child->GetGUID());
		}

		// Recursively Save/Update child Prefabing
		for (Entity child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(child))
		{
			SavePrefabChild(child, newPrefab, mainPrefabGUID);
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
		m_TempPrefab->GetComponent<Prefabing>().m_Instances.emplace(instance->GetGUID());
		Prefabing& tempPrefab{ m_TempPrefab->GetComponent<Prefabing>() };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_PrefabGUID = tempPrefab.m_PrefabGUID;				// Assign it's m_PrefabGUID to m_PrefabGUID
		//instPrefab.m_Instances.clear();									// It is an instance, it does not have any instances
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten
		instPrefab.m_Instances = tempPrefab.m_Instances;				// Copy instances

		Entity mainTempPrefab = m_TempPrefab;

		// Do the same for the children
		std::vector<std::string> children{ instance->GetComponent<Parenting>().m_Children };
		for (size_t i{}; i < children.size(); ++i) // Doing this instead cos string might be too long and cos errors :(
		{
			std::string childID{ children[i] };
			CreatePrefabChild(childID, instance);
		}

		m_TempPrefab = mainTempPrefab;

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

	std::unordered_map<std::string, Entity> PrefabSystem::GetPrefabEntity(std::string prefabFilePath)
	{
		entt::registry copy;
		PrefabInputArchive arc(prefabFilePath, copy);

		// REMEMBER TO UPDATE Prefab.cpp TOO!!!
		entt::basic_snapshot_loader loader(copy);
		loader.entities(arc)
			.component<Prefabing>(arc)
			.component<Parenting>(arc)
			.component<Properties>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<Rigidbody>(arc)
			.component<FEL>(arc)
			.component<FAKEFEL>(arc)
			;

		// Clone each component of the object into the prefab
		// USES MEMORYMANAGER INSTEAD OF ECSMANAGER, ECSManager should never know this exist!
		std::string name{};
		unsigned int count{};
		copy.each([&](entt::entity srcEntity) {
			++count;	// Add count
			Entity entity = MemoryManager::Instance().GetUndeployedEntity();
			for (auto [id, source_storage] : copy.storage())
			{
				auto destination_storage = ECSManager::Instance().GetRegistry().storage(id);
				if (destination_storage != nullptr && source_storage.contains(srcEntity))
				{
					if (!destination_storage->contains(entity->m_Entity))
					{
						destination_storage->emplace(entity->m_Entity, source_storage.get(srcEntity));
					}
					// Overwrite m_Entity if m_Entity already contains the component
					else
					{
						destination_storage->erase(entity->m_Entity);
						destination_storage->emplace(entity->m_Entity, source_storage.get(srcEntity));
					}
				}
			}
			std::string entGUID{ copy.get<Properties>(srcEntity).m_GUID };
			name = { copy.get<Properties>(srcEntity).m_Name };
			m_TempPrefabs.emplace(entGUID, entity);
			m_TempPrefab = entity;
		});

		// Throw error if m_TempPrefab does not even have Prefabing
		if (!m_TempPrefab->HasComponent<Prefabing>())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + m_TempPrefab->GetName() + " does not even have Component Prefabing! Therefore it is invalid!");
			ResetTempPrefab();
			assert(m_TempPrefab->HasComponent<Prefabing>());
		}

		return m_TempPrefabs;
	}

	bool PrefabSystem::UpdatePrefabEntity()
	{
		if (!m_TempPrefab)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] m_TempPrefab is a nullptr!");
			return false;
		}

		// Ensure Prefab exist
		Entity MainPrefab{ m_TempPrefab };

		for (auto prefabPair : m_TempPrefabs)
		{
			m_TempPrefab = prefabPair.second;
			Prefabing& prefabComp{ m_TempPrefab->GetComponent<Prefabing>() };
			std::string prefabGUID{ prefabComp.m_PrefabGUID };

			UpdateAllInstances(prefabComp.m_Instances, prefabGUID);
		}

		m_TempPrefab = MainPrefab;
		std::string prefabGUID{ m_TempPrefab->GetComponent<Prefabing>().m_PrefabGUID };
		if (m_ExistingPrefabs.empty() || m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] prefabGUID (" + prefabGUID + ") is invalid!");
			assert(!m_ExistingPrefabs.empty());
			assert(m_ExistingPrefabs.find(prefabGUID) != m_ExistingPrefabs.end());
		}

		std::string filePath{ m_ExistingPrefabs[prefabGUID] };

		entt::registry tmp;

		SetUpRegistry(tmp);

		// Clone Prefab
		UpdateEntityInRegistry(m_TempPrefab, tmp);

		// GetNoOfEntities
		int noOfEntities{};
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetTotalEntities(noOfEntities, MainPrefab);

		SerializePrefabOutputArchive(tmp, prefabGUID, filePath, noOfEntities);

		// Release m_TempPrefab
		ResetTempPrefab();

		return true;
	}

	void PrefabSystem::SaveEntityInRegistry(Entity object, entt::registry& dstReg, std::string parentGUID, entt::entity parentEnt)
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
		for (Entity child : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(object))
		{
			SaveEntityInRegistry(child, dstReg, entGUID, ent);
		}
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

	void PrefabSystem::CreatePrefabChild(std::string childGUID, Entity& parent)
	{
		// Create an Entity from ECSManager
		Entity childTemp{ m_TempPrefabs[childGUID] };
		Entity instance{ ECSManager::Instance().CloneEntity(childTemp, childTemp->GetName()) };

		// Increment it's instance by one and add this in
		childTemp->GetComponent<Prefabing>().m_Instances.emplace(instance->GetGUID());
		Prefabing& tempPrefab{ childTemp->GetComponent<Prefabing>() };

		// Update the Prefab component to make sense
		Prefabing& instPrefab{ instance->GetComponent<Prefabing>() };
		instPrefab.m_PrefabGUID = tempPrefab.m_PrefabGUID;				// Assign it's m_PrefabGUID to m_PrefabGUID
		//instPrefab.m_Instances.clear();									// It is an instance, it does not have any instances
		instPrefab.m_Overrides.clear();									// It is a newborn, it does not have any overwritten
		instPrefab.m_Instances = tempPrefab.m_Instances;				// Copy instances

		// Update parent and child relationship by assigning the correct guid
		std::vector<std::string>& parentsChildren{ parent->GetComponent<Parenting>().m_Children };
		auto it = std::find(parentsChildren.begin(), parentsChildren.end(), childTemp->GetGUID());
		parentsChildren.erase(it);
		parentsChildren.emplace_back(instance->GetGUID());
		instance->GetComponent<Parenting>().m_Parent = parent->GetGUID();

		// Do the same for the children
		std::vector<std::string> children{ instance->GetComponent<Parenting>().m_Children };
		for (size_t i{}; i < children.size(); ++i) // Doing this instead cos string might be too long and cos errors :(
		{
			std::string childID{ children[i] };
			CreatePrefabChild(childID, instance);
		}
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
		std::ifstream file;
		file.open(filePathName);
		if (!file)
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
		// Update all instances to match
		//std::vector<std::string> invalidInstance;
		for (const std::string& str : instanceGUID)
		{
			std::string instanceID{ str };
			Entity instance{ ECSManager::Instance().FindEntity(instanceID) };
			if (instance && UpdateInstance(instance, prefabGUID))
			{
				instance->GetComponent<Transform>().m_IsDirty = true;
			}
			/*if (!ECSManager::Instance().FindEntity(instanceID) || !UpdateInstance(ECSManager::Instance().FindEntity(instanceID), prefabGUID))
			{
				invalidInstance.emplace_back(instanceID);
			}*/
		}

		//// Erase invalid instance from prefab
		//for (std::string& instanceID : invalidInstance)
		//{
		//	instanceGUID.erase(std::find(instanceGUID.begin(), instanceGUID.end(), instanceID));
		//}
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
		instance->GetComponent<Prefabing>().m_Instances = m_TempPrefab->GetComponent<Prefabing>().m_Instances;

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
				MemoryManager::Instance().ReleaseDeployedEntity(static_cast<ENTTID>(entityPair.second->m_Entity));
			}
			m_TempPrefabs.clear();
			m_TempPrefab = nullptr;
		}
	}
}