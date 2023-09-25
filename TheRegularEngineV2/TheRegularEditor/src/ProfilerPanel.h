/*!
	@file      ProfilerPanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      10/09/2023
	@brief     Profiler panel

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
	class ProfilerPanel : public Panel
	{
	public:
		ProfilerPanel();
		~ProfilerPanel();
		void Init() override;
		void Update() override;
		void Shutdown() override;
		void addFps(const float& fps);
		void ReceiveTimeTaken(const SendTimeTakenEvent& event);

	private:
		std::array<float, 20> m_FpsInfo{};
		int m_FpsInfoIndex{};
		float m_AvgFps{};

		float m_TotalTime{};
		float m_RenderTime{};
		float m_PhysicsTime{};
		float m_ImguiTime{};
		//To be done
		float m_ScriptTime{};
		std::list<std::string> m_ProfilerStringList{};

	};
}