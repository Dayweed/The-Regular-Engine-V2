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

		const std::filesystem::path path{ m_FileName };
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
		// Getting ID of entity
		uint32_t entID = m_Current[m_CurrentIdx].get<uint32_t>();
		ent = entt::entity(entID);
		m_CurrentIdx++;
	}

	void PrefabInputArchive::operator()(std::underlying_type_t<entt::entity>& u)
	{
		// Getting component
		m_RootIdx++;
		if (m_RootIdx >= m_Root.size())
		{
			std::string funcName{ __FUNCTION__ };
			std::string error{ "[" + funcName + "] InputArchive have m_RootIdx " + std::to_string(m_RootIdx) + " < " + std::to_string(m_Root.size())
				+ "! This means there is a new component! Assuming it doesn't have it..."};
			TRE_CORE_WARN(error);
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
		// Update Prefab Directory if there is invalid data
		if (DeserializePrefabDirectory()) SerializePrefabDirectory();
	}

	void PrefabSystem::Update()
	{

	}

	void PrefabSystem::AfterReset()
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
			//MemoryManager::Instance().ClearUndeployed();

			// Save the registry
			GameLoop::Instance().GetBackUpRegistry().clear();
			ECSManager::Instance().SaveRegistry(GameLoop::Instance().GetBackUpRegistry());
		}

		GameLoop::Instance().SetDisplayingPrefab(true);

		// Mimick Game Loop when forcing the scene to be resetted
		ECSSystemManager::Instance().BeforeReset();

		// Clear the "scene" and show the displayed prefab
		ECSManager::Instance().DestroyAll();

		// Create Mandotary Main Camera
		Entity MainCamera = ECSManager::Instance().CreateEntity("Main Camera");
		MainCamera->AddComponent<Camera>();
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(MainCamera, true);

		m_DisplayedPrefab = nullptr;
		m_DisplayedPrefab = ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(prefabGUID);

		ECSSystemManager::Instance().AfterReset();

		return m_DisplayedPrefab;
	}

	void PrefabSystem::ReturnToScene()
	{
		m_DisplayedPrefab = nullptr;

		// Mimick Game Loop when forcing the scene to be resetted
		ECSSystemManager::Instance().BeforeReset();

		// Copy registry and components
		ECSManager::Instance().CopyRegistry(GameLoop::Instance().GetBackUpRegistry());
		// Clear Backup
		GameLoop::Instance().GetBackUpRegistry().clear();
		// Update all Prefabs
		CheckAndUpdateInstances();
		// Auto set back to false
		GameLoop::Instance().SetDisplayingPrefab(false);

		ECSSystemManager::Instance().AfterReset();
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

				//UpdateAllInstances(prefabComp.m_Instances, prefabGUID);
				UpdateAllInstances(prefabGUID);
			}

			ResetTempPrefab();
		}
	}

	Entity PrefabSystem::GetDisplayedPrefab()
	{
		return m_DisplayedPrefab;
	}

	bool PrefabSystem::IsValidPrefabResource(std::string prefabGUID)
	{
		DeserializePrefabDirectory();

		// Find if it exists in existingPrefabs
		if (m_ExistingPrefabs.find(prefabGUID) == m_ExistingPrefabs.end())
		{
			return false;
		}

		std::string prefabID{ prefabGUID };
		std::string prefabPath{ m_ExistingPrefabs[prefabGUID] };

		// Try opening filePath
		std::ifstream file(prefabPath);
		if (!file.is_open())
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_WARN("[" + funcName + "] PrefabDirectory GUID (" + prefabID + ") does not have a valid filepath (" + prefabPath + ")! Removing from m_ExistingPrefabs...");
			return false;
		}
		file.close();

		// Can open file, so it is valid
		return true;
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

			prefabGUID = MemoryManager::Instance().GenerateGUIDStr();
			filePath = FILESYS_PREFABRSCFOLDER + object->GetName() + FILESYS_PREFABRSCTYPE;
			object->AddComponent<Prefabing>().m_PrefabGUID = prefabGUID;
			object->GetComponent<Prefabing>().m_MainPrefabGUID = prefabGUID;
			// Add as GUID
			//object->GetComponent<Prefabing>().m_Instances.emplace(object->GetGUID());

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
		EntityCopier::Instance().SaveEntityInRegistry(object, tmp);

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

				//UpdateAllInstances(prefabComp.m_Instances, prefabComp.m_PrefabGUID);
				UpdateAllInstances(prefabComp.m_PrefabGUID);
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
			Rigidbody,
			SphereCollider,
			BoxCollider,
			CapsuleCollider,
			Audio,
			AudioListener,
			DirectionalLight,
			ScriptComponent,
			UIComponent,
			AnimationComponent,
			ParticleComponent,
			DirectPathfinding,
			TextComponent,
			CylinderCollider,
			SlideshowComponent
		>();
	}

	std::string PrefabSystem::SerializePrefabOutputArchive(entt::registry& reg, std::string prefabGUID, std::string filePath, int NoOfEntities)
	{
		// Ensure prefab folder
		std::filesystem::create_directory(FILESYS_PREFABDEFFOLDER);

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
			.component<Rigidbody>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<CapsuleCollider>(arc)
			.component<Audio>(arc)
			.component<AudioListener>(arc)
			.component<DirectionalLight>(arc)
			.component<ScriptComponent>(arc)
			.component<UIComponent>(arc)
			.component<AnimationComponent>(arc)
			.component<ParticleComponent>(arc)
			.component<DirectPathfinding>(arc)
			.component<TextComponent>(arc)
			.component<CylinderCollider>(arc)
			.component<SlideshowComponent>(arc)
			;

		arc.Close();

		reg.clear();

		// Update Prefab Directory
		UpdatePrefabDirectory(prefabGUID, arc.GetFilePath());

		return arc.GetFilePath();
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
			.component<Rigidbody>(arc)
			.component<SphereCollider>(arc)
			.component<BoxCollider>(arc)
			.component<CapsuleCollider>(arc)
			.component<Audio>(arc)
			.component<AudioListener>(arc)
			.component<DirectionalLight>(arc)
			.component<ScriptComponent>(arc)
			.component<UIComponent>(arc)
			.component<AnimationComponent>(arc)
			.component<ParticleComponent>(arc)
			.component<DirectPathfinding>(arc)
			.component<TextComponent>(arc)
			.component<CylinderCollider>(arc)
			.component<SlideshowComponent>(arc)
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
}