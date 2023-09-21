#include "pch.h"
#include "Prefab.h"
#include "Resource/Resource.h"
#include "TREIncludes.h"

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

	void PrefabManager::Update()
	{

	}

	void PrefabManager::OnDestroyGO()
	{

	}

	void PrefabManager::Shutdown()
	{

	}

	std::string PrefabManager::SavePrefabEntity(Entity object)
	{
		std::string prefab_guid{ object->GetComponent<Properties>().m_GUID = MemoryManager::Instance().GenerateGUIDStr() };

		//std::string prefabGUID{ Resource::GetGUIDHex(Resource::GenerateGUID()) };
		std::string prefabGUID{ prefab_guid };

		std::string filePath{ "../Resources/Prefabs/" + prefabGUID + ".json" };

		entt::registry tmp;

		tmp.view<
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
			.component<Properties>(arc)
			.component<Parenting>(arc)
			.component<Transform>(arc)
			.component<MeshRenderer>(arc)
			.component<Camera>(arc)
			.component<FEL>(arc);
			;

		arc.Close();

		return arc.GetFilePath();
	}
}