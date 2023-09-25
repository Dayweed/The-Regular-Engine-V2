/*!
	@file      ConsolePanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.tan.d@digipen.edu
	@date      02/09/2023
	@brief     Console panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "TREIncludes.h"
#include "Panel.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	class ConsolePanel : public Panel
	{
		public:
			ConsolePanel();
			~ConsolePanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;
			void OnConsole(ConsoleDebugEvent& event);

		private:
			std::vector<std::string> m_ConsoleLog;
			std::vector<std::string> m_ConsoleTimestamp;
			bool m_AutoScroll = true;
			bool m_AutoClear = true;
	};
}