#pragma once
#include "pch.h"
#include "ProfilerPanel.h"
#include "Imgui/imgui.h"
#include <EventSystem/EventHandler/EventHandler.h>
#include "implot.h"

namespace TRE
{
	ProfilerPanel::ProfilerPanel()
	{
		
	}

	ProfilerPanel::~ProfilerPanel()
	{

	}

	void ProfilerPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &ProfilerPanel::ReceiveTimeTaken);
	}

	void ProfilerPanel::Update()
	{
		if (ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			if(Engine::GetInstance().GetWindow()->GetDeltaTime() != 0)
				addFps(1 / Engine::GetInstance().GetWindow()->GetDeltaTime());
			const std::string avgFpsString = "Average FPS: " + std::to_string(static_cast<int>(m_AvgFps));
			ImGui::PlotLines("FPS", m_FpsInfo.data(), static_cast<int>(m_FpsInfo.size()), 0, avgFpsString.c_str(), 0.f, 80.f, ImVec2(300.f, 20.f));

			m_TotalTime = (float)Profiler::Instance().GetTotalTime();
			ImGui::Text("Total Time: %.2f microseconds", m_TotalTime);

			ImGui::Checkbox("Show Real Time Graph", &m_OnPlot);
			if(m_OnPlot)
				PlotRealTimeGraph();

			//PlotRealTimeGraph();
		}
		ImGui::End();
	}

	void ProfilerPanel::Shutdown()
	{

	}

	void ProfilerPanel::addFps(const float& fps)
	{
		const int nextIndex = m_FpsInfoIndex + 1;
		if(nextIndex > m_FpsInfo.size())
			m_FpsInfoIndex = 0;
		m_FpsInfo[m_FpsInfoIndex++] = fps;

		const int fpsSize = static_cast<int>(m_FpsInfo.size());
		for (const float& i : m_FpsInfo)
			m_AvgFps += i;
		m_AvgFps /= static_cast<float>(fpsSize);
	}

	void ProfilerPanel::ReceiveTimeTaken(const SendTimeTakenEvent& event)
	{
		m_ProfiledData = event.m_Timers;
	}

	void ProfilerPanel::PlotRealTimeGraph()
	{
		static float t = 0;
		t += ImGui::GetIO().DeltaTime;
		if(m_ProfiledData.empty())
			return;

		std::vector<std::string> labels;
		for (auto& [key, val] : m_ProfiledData)
		{
			labels.push_back(key);
			if (m_BufferMap.find(key) == m_BufferMap.end())
			{
				m_BufferMap[key] = RollingBuffer();
			}
		}

		for (auto& [key, val] : m_BufferMap)
		{
			m_BufferMap[key].AddPoint(t, (float)m_ProfiledData[key]->GetTime());
		}

		static ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
		if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 0)))
		{
			ImPlot::SetupAxes("Time(s)", nullptr, flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1000);
			ImPlot::SetupLegend(ImPlotLocation_NorthEast);

			for (int i{}; i < labels.size(); ++i)
				ImPlot::PlotLine(labels[i].data(), &m_BufferMap[labels[i]].m_Data[0].x, &m_BufferMap[labels[i]].m_Data[0].y,
					m_BufferMap[labels[i]].m_Data.size(), 0, 0, 2 * sizeof(float));
			ImPlot::EndPlot();
		}
	}
}