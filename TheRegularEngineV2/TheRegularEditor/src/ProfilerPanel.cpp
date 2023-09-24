#pragma once
#include "pch.h"
#include "ProfilerPanel.h"
#include "Imgui/imgui.h"
#include <EventSystem/EventHandler/EventHandler.h>

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
			const float otherTime = m_TotalTime - (m_RenderTime + m_PhysicsTime + m_ScriptTime);

			ImGui::Text("Total Time: %.4f ns", m_TotalTime);
			ImGui::Text("Render Time: %.4f ns", m_RenderTime);
			ImGui::Text("Physics Time: %.4f ns", m_PhysicsTime);
			ImGui::Text("Script Time: %.4f ns", m_ScriptTime);
			ImGui::Text("Imgui Time: %.4f ns", m_ImguiTime);
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
		float count = static_cast<float>(event.m_ms.count());
		count /= 1000.f;
		switch (event.m_type)
		{
		case TimerType::VKRENDER:
			m_RenderTime = count;
			break;
		case TimerType::PHYSICS:
			m_PhysicsTime = count;
			break;
		case TimerType::SCRIPTS:
			m_ScriptTime = count;
			break;
		case TimerType::IMGUI:
			m_ImguiTime = count;
			break;
		default:
			break;
		}	
	}
}