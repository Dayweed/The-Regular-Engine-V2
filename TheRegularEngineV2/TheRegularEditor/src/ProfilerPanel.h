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

	private:
		std::array<float, 20> m_FpsInfo{};
		int m_FpsInfoIndex{};
		float m_AvgFps{};
		float m_TotalTime{};
		float m_RenderTime{};
		float m_PhysicsTime{};

	};
}