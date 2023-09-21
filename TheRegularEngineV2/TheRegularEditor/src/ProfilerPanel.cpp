
#include "pch.h"
#include "ProfilerPanel.h"
#include "Imgui/imgui.h"

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

	}

	void ProfilerPanel::Update()
	{
		ImGui::Begin("Profiler");
		std::stringstream Fps;
		Fps << 1/Engine::GetInstance().GetWindow()->GetDeltaTime() << "FPS\n";
		//Profiler::GetTimers()
		//ImGui::PlotLines()
		std::stringstream physics;
		physics << Profiler::Instance().GetTimers()[typeid(MeshRendererSystem).name()]->GetTime() << "ms\n";
		ImGui::TextUnformatted(physics.str().c_str());

		ImGui::End();
	}

	void ProfilerPanel::Shutdown()
	{

	}
}