#pragma once
#include "PhysX/PxPhysicsAPI.h"

namespace TRE
{
	class ErrorCallback : public physx::PxErrorCallback
	{
		void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};
}
