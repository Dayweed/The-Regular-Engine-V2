#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class Panel
	{
		public:
			Panel() {};
			virtual ~Panel() {};
			virtual void Init() = 0;
			virtual void Update() = 0;
			virtual void Shutdown() = 0;

		private:
			std::string PanelName;
	};
}