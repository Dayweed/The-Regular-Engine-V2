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
			addFps(1 / Engine::GetInstance().GetWindow()->GetDeltaTime());
			const std::string avgFpsString = "Average FPS: " + std::to_string(static_cast<int>(m_AvgFps));
			ImGui::PlotLines("FPS", m_FpsInfo.data(), static_cast<int>(m_FpsInfo.size()), 0, avgFpsString.c_str(), 0.f, 80.f, ImVec2(300.f, 20.f));

			m_TotalTime = Profiler::Instance().GetTotalTime();
			ImGui::Text("Total Time: %.2f microseconds", m_TotalTime);
			ImGui::Text("Percentages for each system by order its called:");

			const std::string str = "class TRE::";
			for (auto& i : m_ProfilerStringList)
			{
				if (i.find(str) != std::string::npos)
				{
					i.erase(0, str.size());
				}
				ImGui::TextUnformatted(i.c_str());
			}
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
		float avgFps = 0.f;
		for (const float& i : m_FpsInfo)
			m_AvgFps += i;
		m_AvgFps /= static_cast<float>(fpsSize);
	}

	void ProfilerPanel::ReceiveTimeTaken(const SendTimeTakenEvent& event)
	{
		m_ProfilerStringList = event.m_Str;
	}
}