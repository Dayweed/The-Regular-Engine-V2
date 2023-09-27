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
		float m_ScriptTime{};
		//To activate the profiler
		bool m_OnPlot{ false };
		//For stoopid ImPlot
		std::unordered_map<std::string, Timer*>m_ProfiledData;

		//Not using scrolling buffer so I just hide it
		/*struct ScrollingBuffer
		{
			ScrollingBuffer(const int size = 2000)
			{
				m_MaxSize = size;
				m_Offset = 0;
				m_Data.reserve(m_MaxSize);
			}
			~ScrollingBuffer() = default;

			void AddPoint(const float x, const float y)
			{
				if(m_Data.size() < m_MaxSize)
				{
					m_Data.push_back(ImVec2(x, y));
				}
				else
				{
					m_Data[m_Offset] = ImVec2(x, y);
					m_Offset = (m_Offset + 1) % m_MaxSize;
				}
			}

			void Erase()
			{
				if (!m_Data.empty())
				{
					m_Data.shrink(0);
					m_Offset = 0;
				}
			}

			int m_MaxSize;
			int m_Offset;
			ImVector<ImVec2> m_Data;
		};*/

		struct RollingBuffer
		{
			RollingBuffer()
			{
				m_Span = 10.f;
				m_Data.reserve(2000);
			}

			void AddPoint(float x, float y)
			{
				const float xmod = fmodf(x, m_Span);
				if (!m_Data.empty() && xmod < m_Data.back().x)
					m_Data.shrink(0);
				m_Data.push_back(ImVec2(xmod, y));
			}

			float m_Span;
			ImVector<ImVec2> m_Data;
		};

		std::unordered_map<std::string, RollingBuffer> m_BufferMap{};
		void PlotRealTimeGraph();
	};
}