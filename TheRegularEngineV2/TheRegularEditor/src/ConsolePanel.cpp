#include "pch.h"
#include "ConsolePanel.h"
#include "Imgui/imgui.h"

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

	}
	
	void ConsolePanel::Update()
	{
		ImGui::Begin("Console");
		ImGui::End();
	}

	void ConsolePanel::Shutdown()
	{

	}
}