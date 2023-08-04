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

	void ECSManager::UpdateAll()
	{
		auto view = registry.view<Position>();

		for (auto obj : view)
		{
			auto& pos = view.get<Position>(obj);
		}

		auto updview = registry.view<TESTUpdate>();

		for (auto obj : updview)
		{
			auto& upd = updview.get<TESTUpdate>(obj);
			upd.Update();
		}
	}

	void ECSManager::DestroyAll()
	{
		registry.clear();
	}

	GO ECSManager::CreateGO()
	{
		GO obj{ std::make_unique<GameObject>() };
		obj->entity = registry.create();
		return obj;
	}

	void ECSManager::DestroyGO(GO& object)
	{
		// Release all components and entity itself
		registry.destroy(object->entity);
		// Free unique ptr from the object
		object.release();
	}
}