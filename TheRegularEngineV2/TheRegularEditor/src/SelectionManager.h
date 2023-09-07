#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class SelectionManager
	{
		public:
			SelectionManager();
			~SelectionManager();
			bool IsEntitySelected(Entity& EntityObject);
			Entity& GetSelectedEntity();
			void SelectEntity(Entity& EntityObject);

		private:
			Entity m_SelectedEntity;
	};
}