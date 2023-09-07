#include "pch.h"
#include "ConsolePanel.h"
#include "Imgui/imgui.h"
#include "EventSystem/EventHandler/EventHandler.h"

namespace TRE
{
	ConsolePanel::ConsolePanel()
	{

	}

	ConsolePanel::~ConsolePanel()
	{

	}

	void ConsolePanel::Init()
	{
        EventHandler::getEventHandlerInstance().subscribe(this, &ConsolePanel::OnConsole);
	}
	
	void ConsolePanel::Update()
	{
		if (ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
			ImGui::SameLine();
			ImGui::Checkbox("Auto-clear", &m_AutoClear);
			ImGui::SameLine();
			if (ImGui::Button("Clear##Console"))
			{
				m_ConsoleLog.clear();
			}
            if (ImGui::BeginChild("CONSOLE AREA", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
            {
                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(m_ConsoleLog.size()));
                while (clipper.Step())
                {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    {
                        ImVec4 color;
                        bool has_color = false;

                        if (m_ConsoleLog[i].starts_with(std::string_view("[ERROR]")))
                        {
                            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                            has_color = true;
                        }
                        if (m_ConsoleLog[i].starts_with(std::string_view("[WARNING]")))
                        {
                            color = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
                            has_color = true;
                        }

                        ImGui::TextUnformatted(m_ConsoleTimestamp[i].c_str());

                        if (has_color)
                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::SameLine();
                        ImGui::TextUnformatted(m_ConsoleLog[i].c_str());
                        if (has_color)
                            ImGui::PopStyleColor();
                    }
                }
            }
            if (m_AutoScroll && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
        }
		ImGui::End();
	}

    void ConsolePanel::OnConsole(ConsoleDebugEvent& event)
    {
        time_t now = time(0);
        const tm* localTime = localtime(&now);
        std::stringstream ss{};
        ss << "[" << (localTime->tm_hour) << ":" << std::setw(2) << std::setfill('0') << (localTime->tm_min) << ":" << (localTime->tm_sec) << "]";
        m_ConsoleTimestamp.emplace_back(ss.str().c_str());
        m_ConsoleLog.emplace_back(event.m_Msg.c_str());
    }

	void ConsolePanel::Shutdown()
	{

	}
}