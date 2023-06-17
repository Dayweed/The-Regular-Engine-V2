#pragma once
#include "Core/Layer.h"

namespace TRE
{
	class PhysicsSystem : public Layer
	{
		public:
			PhysicsSystem();
			~PhysicsSystem();

			void Update() override;
			void Shutdown() override;

		private:
	};
}
