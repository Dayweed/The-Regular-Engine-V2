/*!
	@file      Panel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      02/09/2023
	@brief     Panel parent class

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
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