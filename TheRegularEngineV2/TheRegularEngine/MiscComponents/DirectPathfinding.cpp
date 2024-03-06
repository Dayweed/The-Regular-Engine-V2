#include "pch.h"
#include "ECS/Components/DirectPathfinding.h"
#include "ECS/SystemManager.h"
#include "TREIncludes.h"

namespace TRE
{
	void DirectPathfindingSystem::GameUpdate()
	{
		// Auto start any pathfinding that is set to run on start
		if (m_SceneStart)
		{
			for (Entity& go : ECSManager::Instance().GetEntities<Transform, DirectPathfinding>())
			{
				DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };

				if (path.m_StartOnPlay)
				{
					StartPathfinding(go);
				}
			}

			m_SceneStart = false;
		}

		// Check for movement
		for (Entity& go : ECSManager::Instance().GetEntities<Transform, DirectPathfinding>())
		{
			// Move current position to the next index
			Transform& transform{ go->GetComponent<Transform>() };
			DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
			path.m_OldPosition = transform.m_Position;

			// Ignores if it is not running
			if (!path.m_IsRunning) continue;

			// Ignores if there is none to traverse to
			if (path.m_WayPoints.empty()) continue;

			// Ignores if index is out of range
			if (path.m_CurrentIndex < 0 || path.m_CurrentIndex >= path.m_WayPoints.size()) continue;

			glm::vec3 dir = path.m_WayPoints[path.m_CurrentIndex].m_Value - transform.m_Position;
			glm::vec3 normDir{0,0,0};
			if (glm::length(dir) > 0)
			{
				normDir = glm::normalize(dir);
			}

			// Lerps through each positions
			if (path.m_CurrentTime <= 0.0f)
			{
				glm::vec3 ww{ normDir * path.m_Speed * Engine::GetInstance().GetWindow()->GetDeltaTime() };
				transform.m_Position += normDir * path.m_Speed * Engine::GetInstance().GetWindow()->GetDeltaTime();
				transform.m_DirtyFlags |= TransformDirtyFlags::TRE_DIRTY_POSITION;
				transform.m_IsDirty = true;

				// Move to next index, if it is very close to the ideal position
				if (IsNearPosition(go, path.m_WayPoints[path.m_CurrentIndex].m_Value))
				{
					if (path.m_Direction) ++path.m_CurrentIndex; else --path.m_CurrentIndex;
					if ((path.m_CurrentIndex >= path.m_WayPoints.size() || path.m_CurrentIndex < 0) && path.m_Repeat)
					{
						if (!path.m_Reverse)
						{
							path.m_CurrentIndex = 0;
						}
						else
						{
							path.m_CurrentIndex = path.m_CurrentIndex < 0 ? 0 : static_cast<int>(path.m_WayPoints.size() - 1);
							path.m_Direction = !path.m_Direction;
						}
					}
					path.m_CurrentTime = path.m_Delay;
				}
			}
			else
			{
				path.m_CurrentTime -= Engine::GetInstance().GetWindow()->GetDeltaTime();
			}
		}
	}

	void DirectPathfindingSystem::AfterReset()
	{
		m_SceneStart = true;
	}

	void DirectPathfindingSystem::ResetPathfinding(Entity go)
	{
		if (!go->HasComponent<DirectPathfinding>()) return;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		path.m_CurrentIndex = 0;
		path.m_CurrentTime = path.m_Delay;
		path.m_IsRunning = false;
	}

	void DirectPathfindingSystem::StartPathfinding(Entity go)
	{
		if (!go->HasComponent<DirectPathfinding>()) return;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		path.m_CurrentIndex = 0;
		path.m_CurrentTime = path.m_Delay;
		path.m_IsRunning = true;
	}

	void DirectPathfindingSystem::TogglePausePathfinding(Entity go)
	{
		if (!go->HasComponent<DirectPathfinding>()) return;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		path.m_IsRunning = !path.m_IsRunning;
	}

	void DirectPathfindingSystem::AddPoint(Entity go, glm::vec3 point)
	{
		if (!go->HasComponent<DirectPathfinding>()) return;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		waypoint wp{};
		wp.m_Value = point;
		path.m_WayPoints.push_back(wp);
	}

	bool DirectPathfindingSystem::RemoveIndex(Entity go, int index)
	{
		if (!go->HasComponent<DirectPathfinding>()) return false;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		if (index >= path.m_WayPoints.size()) return false;

		path.m_WayPoints.erase(path.m_WayPoints.begin() + index);
		return true;
	}

	bool DirectPathfindingSystem::MoveIndex(Entity go, int currIndex, int newIndex)
	{
		if (!go->HasComponent<DirectPathfinding>()) return false;

		DirectPathfinding& path{ go->GetComponent<DirectPathfinding>() };
		if (currIndex >= path.m_WayPoints.size()) return false;
		if (newIndex >= path.m_WayPoints.size()) return false;

		waypoint pos{ path.m_WayPoints[currIndex] };
		path.m_WayPoints.erase(path.m_WayPoints.begin() + currIndex);
		path.m_WayPoints.insert(path.m_WayPoints.begin() + newIndex, pos);
		return true;
	}

	bool DirectPathfindingSystem::IsNearPosition(Entity go, glm::vec3 position)
	{
		Transform& transform{ go->GetComponent<Transform>() };

		glm::vec3 minusOffset = position - m_Offset;
		glm::vec3 plusOffset = position + m_Offset;

		glm::vec3 minOffset = glm::vec3(
			(minusOffset.x < plusOffset.x) ? minusOffset.x : plusOffset.x,
			(minusOffset.y < plusOffset.y) ? minusOffset.y : plusOffset.y,
			(minusOffset.z < plusOffset.z) ? minusOffset.z : plusOffset.z
		);

		glm::vec3 maxOffset = glm::vec3(
			(minusOffset.x > plusOffset.x) ? minusOffset.x : plusOffset.x,
			(minusOffset.y > plusOffset.y) ? minusOffset.y : plusOffset.y,
			(minusOffset.z > plusOffset.z) ? minusOffset.z : plusOffset.z
		);

		return transform.m_Position.x >= minOffset.x && transform.m_Position.x <= maxOffset.x
			&& transform.m_Position.y >= minOffset.y && transform.m_Position.y <= maxOffset.y
			&& transform.m_Position.z >= minOffset.z && transform.m_Position.z <= maxOffset.z;
	}
}