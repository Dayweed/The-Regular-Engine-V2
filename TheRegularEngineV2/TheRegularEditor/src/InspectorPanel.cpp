#include "pch.h"
#include "InspectorPanel.h"
#include "Imgui/imgui.h"
#include "TREIncludes.h"

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

		//std::cout << "size of vector: " << ECSManager::Instance().GetEntities<Properties>().size() << "\n";
		//for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
		//{
		//	std::cout << "whats the name: " << ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name << "\n";
		//	
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<Transform>();
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<MeshRenderer>();
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<Camera>();
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<SphereCollider>();
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<BoxCollider>();
		//	ECSManager::Instance().GetEntities<Properties>()[i]->HasComponent<Audio>();
		//}
		//create entity

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
		//ImGui::SameLine();
		ImGui::Text("Layer");
		ImGui::SameLine();
		const char* Layers[] = { "Default" };
		static int LayersIndex = 0; // If the selection isn't within 0..count, Combo won't display a preview
		//ImGui::Combo("##Layer", &LayersIndex, Layers, IM_ARRAYSIZE(Layers));
		if(ImGui::BeginCombo("##Layer", Layers[LayersIndex]))
		{
			for (int n = 0; n < IM_ARRAYSIZE(Layers); n++)
			{
				bool is_selected = (LayersIndex == n);
				if (ImGui::Selectable(Layers[n], is_selected))
					LayersIndex = n;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::End();
	}
	
	void InspectorPanel::Shutdown()
	{

	}
}