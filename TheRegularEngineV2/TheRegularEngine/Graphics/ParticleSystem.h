#pragma once
#include "Core/ECS.h"

namespace TRE
{
	struct Particle
	{
		glm::mat4 L2W;
		glm::vec3 Position;
		glm::vec3 Scale;
	};

	class ParticleSystem : public ECSSystem
	{
		void LateUpdate() override;
	};
}