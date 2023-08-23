#pragma once
#include "pch.h"
#include "entt.hpp"

namespace TRE
{
	class Transform
	{
	public:
		glm::vec3 m_Position{ 0.f,0.f,0.f };
		glm::vec3 m_Rotation{ 0.f,0.f,0.f };
		glm::vec3 m_Scale{ 1.f,1.f,1.f };

		const glm::mat4 GetModelMatrix() const;

		static void Init()
		{
			std::cout << "Transform: Init have been summoned\n";
		}

		static void UpdateValues()
		{
			std::cout << "Transform: UpdateValues have been summoned\n";
		}

		static void Destroy()
		{
			std::cout << "Transform: Destroy  have been summoned\n";
		}
	};
}