#include "CollisionMatrixPanel.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "Physics/PhysicsComponent.h"

namespace TRE
{
	void CollisionMatrixPanel::Init()
	{
		// should this be in the constructor instead?
		EventHandler::getEventHandlerInstance().subscribe(this, &CollisionMatrixPanel::OnCollisionMatrixEvent);
	}

	void CollisionMatrixPanel::Update()
	{
		if (!m_ShowPanel)
			return;

		// green corner grip :D
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, { 0, 1, 0, 1 });

		ImGui::Begin("Collision Matrix", &m_ShowPanel);

		PhysicsSystem* system = ECSSystemManager::Instance().GetSystem<PhysicsSystem>();
		static PhysicsSystem::CollisionMatrix matrix = system->GetCollisionMatrix();
		if (ImGui::Button("Apply Changes"))
		{
			system->SetCollisionMatrix(matrix);
			system->ApplyCollisionMatrix();
			system->SaveCollisionMatrix();
		}

		// green help text :D
		ImGui::TextColored(ImVec4{ 0, 1, 0, 1 }, "For help, hover over this! -->\n");
		ImGui::SameLine();

		static const std::string helpText
		{
			"- Below are the various collision layers, with their acronyms as headers in the first row.\n"
			"- Select the various radio buttons to toggle collision between layers.\n"
			"- Entities without any layer set will be treated as Default.\n"
			"- Double clicking the bottom-right corner of the window (in green) will adjust this window to fit perfectly.\n"
		};
		HelpMarker("(?)", helpText.c_str());
		ImGui::Spacing();

		constexpr int layerNameListSize = CollisionLayer::TOTAL;

		constexpr int tableSize = layerNameListSize + 1;

		// i think i could've set the mode to the standard dark theme, grabbed and saved the colors
		// and returned it to our theme, but i realised this too late. Hardcoded colors it is. :_)

		// Yes this is necessary, these buttons are practically invisible unless I manually give them distinct colors.

		const int newTableStyleStart = __LINE__;
		ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImGui::GetColorU32(ImGuiCol_TableBorderStrong));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::ColorConvertU32ToFloat4(0xFF4F3522)); // from ImGui's Standard Dark Theme
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::ColorConvertU32ToFloat4(0xFFAA5555));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::ColorConvertU32ToFloat4(0xFFEFABAB)); // some lavender thing that looks quite nice actually :)
		const int newTableStyleEnd = __LINE__;

		ImGuiTableFlags tableFlags = ImGuiTableFlags_None;
		tableFlags |= ImGuiTableFlags_Borders;

		if (ImGui::BeginTable("CollisionMatrixTable", tableSize, tableFlags))
		{
			// move 'cursor' to actually enter into the table's first cell
			ImGui::TableNextColumn();

			for (int i = 0; i < tableSize; ++i)
			{
				for (int j = 0; j < tableSize; ++j)
				{
					if (!i && !j)
					{
						// we have a little fun here :)
						const std::string text = "Hello! :D";
						ImVec4 startColor{ 1.0f, 0.0f, 0.0f, 1.0f };
						ImVec4 endColor{ 1.0f, 1.0f, 0.0f, 1.0f };
						ImGui::AlignTextToFramePadding();

						for (unsigned ch = 0; ch < text.size(); ++ch)
						{
							ImVec4 lerpColor = ImLerp(startColor, endColor, static_cast<float>(ch) / text.size());
							ImGui::TextColored(lerpColor, std::string(1, text[ch]).c_str());
							ImGui::SameLine(0, 0);
							// ^ leaving the spacing as the default value of -1 does weird stuff
							// but 0 ensures there are no gaps! :D
						}
					}
					else if (!i) // header row
					{
						ImGui::AlignTextToFramePadding();
						ImGui::SeparatorText(CreateLayerAcronym(j - 1).c_str());
					}
					else if (!j) // left-most column - meant for full layer names
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text(CollisionLayer::m_LayerNameList[i - 1].first.c_str());
					}
					else
					{
						// e.g. "  ##12-1", "  ##6-7"
						const std::string buttonLabel = "  ##" + std::to_string(i - 1) + "-" + std::to_string(j - 1);
						// hacky way to center the buttons - throw some spaces in there!
						ImGui::Text("  ");
						ImGui::AlignTextToFramePadding();
						ImGui::SameLine();

						if (ImGui::RadioButton(buttonLabel.c_str(), matrix[i - 1][j - 1]))
						{
							// arr[i - 1][j - 1] = !arr[i - 1][j - 1];
							matrix[i - 1].flip(j - 1);

							// arr[j - 1][i - 1] = arr[i - 1][j - 1];
							matrix[j - 1].set(i - 1, matrix[i - 1].test(j - 1));
						}

						const std::string hoverText = CreateLayerAcronym(i - 1) + "-" + CreateLayerAcronym(j - 1);
						HoverOverText(hoverText.c_str());
					}
					ImGui::TableNextColumn();
				}
			}
			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::PopStyleColor(newTableStyleEnd - newTableStyleStart - 1); // number of custom styles in the part above
		ImGui::End();
		ImGui::PopStyleColor(); // pop the change in ImGuiCol_ResizeGrip color
	}

	void CollisionMatrixPanel::Shutdown() {}

	void CollisionMatrixPanel::OnCollisionMatrixEvent(const CollisionMatrixEvent& event)
	{
		m_ShowPanel = event.m_ShowCollisionMatrixPanel;
	}

	void CollisionMatrixPanel::HelpMarker(const char* prompt, const char* helpText) const
	{
		ImGui::TextDisabled(prompt);
		HoverOverText(helpText);
	}

	inline void CollisionMatrixPanel::HoverOverText(const char* text) const
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(text);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	std::string CollisionMatrixPanel::CreateLayerAcronym(const int layerID)
	{
		if (layerID == 0)
			return "DEF";

		std::string layerName = CollisionLayer::m_LayerNameList[layerID].first;

		// checks if the layer name doesn't have the form of "LayerXX"
		// if so, use all non-lowercase characters as its acronym
		if (!(layerName.find("Layer") == 0 && layerName.size() == std::string("LayerXX").size()))
			return (std::erase_if(layerName, [](const char ch) {return islower(ch); }), layerName);

		std::stringstream temp;
		temp << std::setw(2) << std::setfill('0') << layerID;
		return "L" + temp.str();
	}
}
