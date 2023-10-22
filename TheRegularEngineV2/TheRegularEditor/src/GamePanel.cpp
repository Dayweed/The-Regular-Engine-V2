#include "GamePanel.h"
#include "Core/Engine.h"

namespace TRE
{
	GamePanel::GamePanel()
	{

	}

	GamePanel::~GamePanel()
	{

	}

	void GamePanel::Init()
	{

	}

	void GamePanel::Update()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Game Viewport");
		ImGui::PopStyleVar();

		m_IsViewportHovered = ImGui::IsWindowHovered();
		m_ImageSize = m_ViewportSize = ImGui::GetContentRegionAvail();
		m_WindowPos = ImGui::GetWindowPos();
		//Window resize -- force to follow 16:9 aspect ratio
		UpdateViewportSize();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetGameSceneDescriptor(), m_ImageSize);

		ImGui::End();
	}

	void GamePanel::Shutdown()
	{

	}

	void GamePanel::UpdateViewportSize()
	{
		const WindowConfig& windowConfig = Engine::GetInstance().GetWindow()->GetWindowConfig();
		const float aspectRatio = static_cast<float>(windowConfig.width) / windowConfig.height;
		if ((m_ViewportSize.x / m_ViewportSize.y) < aspectRatio)
		{
			m_ImageSize.x = m_ViewportSize.x;
			m_ImageSize.y = m_ViewportSize.x / aspectRatio;
		}
		else
		{
			m_ImageSize.y = m_ViewportSize.y;
			m_ImageSize.x = m_ViewportSize.y * aspectRatio;

		}
		//Center the image
		m_ImageOffset = m_ViewportSize - m_ImageSize;
		m_ImageOffset.x *= 0.5f;
		m_ImageOffset.y *= 0.5f;
		m_ImageOffset += ImGui::GetWindowSize() - m_ViewportSize;
		ImGui::SetCursorPos(m_ImageOffset);
	}
}