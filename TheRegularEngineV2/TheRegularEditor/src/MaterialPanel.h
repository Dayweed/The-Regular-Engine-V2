#pragma once
#include "TREIncludes.h"
#include "Panel.h"
#include "SelectionManager.h"
#include "AssetSelector.h"
#include "EventSystem/Events/InputEvent.h"

namespace TRE
{
	class MaterialPanel : public Panel
	{
	public:
		MaterialPanel(const std::shared_ptr<SelectionManager>& selectionManager, const std::shared_ptr<AssetSelector>& assetSelector);
		~MaterialPanel();
		void Init() override;
		void Update() override;
		void Shutdown() override;
	private:
		void DrawTexture(std::pair<const std::string, std::shared_ptr<VulkanTexture>>& texture);
		void Rename(std::shared_ptr<Material> material);
		void OnKeyboardClick(const InputEvent& event);
	private:
		std::shared_ptr<SelectionManager> m_SelectionManager;
		std::shared_ptr<AssetSelector> m_AssetSelector;

		bool m_EnterPressed = false;
	};
}