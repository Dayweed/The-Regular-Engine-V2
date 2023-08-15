#include "pch.h"
#include "ECS.h"

namespace TRE
{
	ECSManager& ECSManager::Instance()
	{
		static ECSManager instance;
		return instance;
	}

	entt::registry& ECSManager::GetRegistry()
	{
		return registry;
	}

	void ECSManager::DestroyAll()
	{
		registry.clear();
	}

	GO ECSManager::CreateGO()
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->entity = registry.create();
		GOList.emplace_back(obj);
		return obj;
	}

	void ECSManager::DestroyGO(GO& object)
	{
		// Remove from GOList
		auto it = std::find_if(GOList.begin(), GOList.end(), [&](GO& go) { return go.get() == object.get(); });
		if (it != GOList.end())
		{
			GOList.erase(it);
		}
		// Release all components and entity itself
		registry.destroy(object->entity);
		// Free unique ptr from the object
		object.reset();
	}

	GO ECSManager::CloneGO(GO& object)
	{
		GO obj{ std::make_shared<GameObject>() };
		obj->entity = registry.create();
		GOList.emplace_back(obj);
		// Clone each component of the object into the clone
		for (auto&& curr : registry.storage())
		{
			std::cout << "A Component Type\n";
			if (auto& storage = curr.second; storage.contains(object->entity))
			{
				std::cout << "	Storage of entities with mentioned component\n";
				std::cout << "	Size of Storage: " << storage.size() << "\n";
				std::cout << "	Cloning Component...\n";
				storage.emplace(obj->entity, storage.get(object->entity));
			}
		}
		// Return clone
		return obj;
	}
}