#include "pch.h"
#include "ErrorCallback.h"

namespace TRE
{
	void ErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		printf("=======================================\n");
		printf("INCOMING PHYSX ERROR:\n");
		printf("Error code was		: |%d|\n", code);
		printf("Error message was	: |%s|\n", message);
		// trust, this makes the indentation look good when printed out
		printf("From file		: |%s|\n", file);
		printf("At line			: |%d|\n", line);
		printf("=======================================\n");
	}
}
