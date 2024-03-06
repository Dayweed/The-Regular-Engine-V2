#include "pch.h"
#include "ECS/Components/Light.h"
#include "ECS/ECS.h"
#include "ECS/Components/Transform.h"

namespace TRE
{
	glm::vec3 DirectionalLight::GetUpVec() const
	{
		glm::vec3 leftVec = glm::normalize(glm::cross(glm::vec3(0,1,0), m_Direction));
		return glm::normalize(glm::cross(leftVec, -m_Direction));
	}

	void LightSystem::LateUpdate()
	{
		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const Transform& transform = entity->GetComponent<Transform>();
			DirectionalLight& light = entity->GetComponent<DirectionalLight>();

			if (transform.m_IsDirty)
			{
				float pitch = glm::radians(transform.m_Rotation.x);
				float yaw = glm::radians(transform.m_Rotation.y);
				float roll = glm::radians(transform.m_Rotation.z);

				glm::mat4 rotationMatrix = glm::mat4(1.0f);
				rotationMatrix = glm::rotate(rotationMatrix, pitch, glm::vec3(1, 0, 0));
				rotationMatrix = glm::rotate(rotationMatrix, yaw, glm::vec3(0, 1, 0));
				rotationMatrix = glm::rotate(rotationMatrix, roll, glm::vec3(0, 0, 1));

				glm::vec4 directionalVec = rotationMatrix * glm::vec4(0, 0, 1, 0);
				light.m_Direction = glm::vec3(directionalVec);
			}
		}
	}
}