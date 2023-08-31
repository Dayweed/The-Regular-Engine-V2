#include "InspectorPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{
	InspectorPanel::InspectorPanel()
	{

	}

	InspectorPanel::~InspectorPanel()
	{

	}

	void InspectorPanel::Init()
	{

	}

	void InspectorPanel::Update()
	{
		ImGui::Begin("Inspector");

		//object shows up in viewport == true; object does not show up in viewport == false
		static bool check = true;
		ImGui::Checkbox("##NoName", &check);
		ImGui::SameLine();

		//object name
		static char str1[128] = "";
		ImGui::InputTextWithHint("##ObjectName", "Object name", str1, IM_ARRAYSIZE(str1));

		//tag
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Tag");
		ImGui::SameLine();
		const char* Tags[] = { "Untagged" };
		static int TagsIndex = 0; // If the selection isn't within 0..count, Combo won't display a preview
		ImGui::Combo("##Tag", &TagsIndex, Tags, IM_ARRAYSIZE(Tags));

		//layer
		ImGui::SameLine();
		ImGui::Text("Layer");
		ImGui::SameLine();
		const char* Layers[] = { "Default" };
		static int LayersIndex = 0; // If the selection isn't within 0..count, Combo won't display a preview
		ImGui::Combo("##Layer", &LayersIndex, Layers, IM_ARRAYSIZE(Layers));

		ImGui::End();
	}
	
	void InspectorPanel::Shutdown()
	{

	}
}