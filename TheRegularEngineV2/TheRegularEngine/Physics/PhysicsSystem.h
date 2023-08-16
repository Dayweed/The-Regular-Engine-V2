#pragma once
#include "Core/System.h"

namespace TRE
{
	class PhysicsSystem : public System
	{
		public:
			PhysicsSystem();
			~PhysicsSystem();

			void Update() override;
			void Shutdown() override;

		private:
	};
}
