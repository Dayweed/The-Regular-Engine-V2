#pragma once
#include "pch.h"
#include "Core/ECS.h"
#include "Core/System.h"

#include <glm/gtc/type_ptr.hpp>

namespace TRE
{
	class DirectPathfinding : property::base
	{
	public:
		std::vector<waypoint> m_WayPoints;

		float m_Delay = 1.f;
		float m_Speed = 1.f;
		int m_CurrentIndex = -1;	// -1: m_WayPoints is empty, in inspector should be disabled

		bool m_StartOnPlay = true;
		bool m_Repeat = true;
		bool m_Reverse = true; // Will loop from start if false [Disabled if m_Repeat is false]

		// Hidden from inspector
		bool m_IsRunning = false;	// Check if it is running

	private:
		float m_CurrentTime = 0.f;	// To track delay timing
		bool m_Direction = true;	// Direction to move index [True: ++; False: --]

		friend class DirectPathfindingSystem;

	public:
		property_vtable()           // Allows the base class to get these properties  

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const DirectPathfinding& t) // Serialize
		{
			// Converting to vector...
			std::vector<float> v_waypoints{};
			for (waypoint pos : t.m_WayPoints)
			{
				v_waypoints.emplace_back(pos.m_Value[0]);
				v_waypoints.emplace_back(pos.m_Value[1]);
				v_waypoints.emplace_back(pos.m_Value[2]);
			}

			j = nlohmann::json{
				{ "m_WayPoints", v_waypoints },
				{ "m_Delay", t.m_Delay },
				{ "m_Speed", t.m_Speed },
				{ "m_CurrentIndex", t.m_CurrentIndex },
				{ "m_Repeat", t.m_Repeat },
				{ "m_Reverse", t.m_Reverse },
				{ "m_StartOnPlay", t.m_StartOnPlay }
			};
		}
		friend void from_json(const nlohmann::json& j, DirectPathfinding& t) // Deserialize
		{
			if (j.contains("m_WayPoints"))
			{
				std::vector<float> v_pos{ j.at("m_WayPoints").get<std::vector<float>>() };
				assert(v_pos.size() % 3 == 0);
				// Properly add the points in
				t.m_WayPoints.clear();
				for (size_t i{}; i < v_pos.size(); i+=3)
				{
					waypoint p{};
					p.m_Value = { v_pos[i], v_pos[i + 1], v_pos[i + 2] };
					t.m_WayPoints.push_back(p);
				}
			}
			if (j.contains("m_Delay"))
			{
				t.m_Delay = j.at("m_Delay").get<float>();
			}
			if (j.contains("m_Speed"))
			{
				t.m_Speed = j.at("m_Speed").get<float>();
			}
			if (j.contains("m_CurrentIndex"))
			{
				t.m_CurrentIndex = j.at("m_CurrentIndex").get<int>();
			}
			if (j.contains("m_Repeat"))
			{
				t.m_Repeat = j.at("m_Repeat").get<bool>();
			}
			if (j.contains("m_Reverse"))
			{
				t.m_Repeat = j.at("m_Reverse").get<bool>();
			}
			if (j.contains("m_StartOnPlay"))
			{
				t.m_StartOnPlay = j.at("m_StartOnPlay").get<bool>();
			}
		}
	};

	class DirectPathfindingSystem : public ECSSystem
	{
	public:
		void GameUpdate() override;
		void AfterReset() override;

		void ResetPathfinding(Entity go);		
		void StartPathfinding(Entity go);		// Same as ResetPathfinding but auto start the pathfinding
		void TogglePausePathfinding(Entity go);

		void AddPoint(Entity go, glm::vec3 point);
		bool RemoveIndex(Entity go, int index);
		bool MoveIndex(Entity go, int currIndex, int newIndex);
		bool IsNearPosition(Entity go, glm::vec3 position);

	private:
		glm::vec3 m_Offset{ 0.1f, 0.1f, 0.1f };
		bool m_SceneStart{ true };
	};
}

property_begin(TRE::DirectPathfinding)
{
		property_var(m_Delay)
		, property_var(m_Speed)
		//, property_var(m_CurrentIndex)
		, property_var_fnbegin("m_CurrentIndex", int)
		{
			if (isRead)
			{
				InOut = Self.m_CurrentIndex;
			}

		} property_var_fnend()
		, property_var(m_StartOnPlay)
		, property_var(m_Repeat)
		, property_var(m_Reverse)
		//, property_var(m_WayPoints)
		, property_var_fnbegin("m_WayPoints", std::vector<waypoint>)
		{
			if (isRead)
			{
				InOut = Self.m_WayPoints;
			}
			else
			{
				Self.m_WayPoints = InOut;
			}

		} property_var_fnend()

} property_vend_h(TRE::DirectPathfinding)