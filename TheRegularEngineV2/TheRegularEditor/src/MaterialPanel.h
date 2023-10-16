#pragma once
#include "TREIncludes.h"
#include "Panel.h"
#include "SelectionManager.h"

namespace TRE
{
	class MaterialPanel : public Panel
	{
	public:
		MaterialPanel(const std::shared_ptr<SelectionManager>& Selection_Manager);
		~MaterialPanel();
		void Init() override;
		void Update() override;
		void Shutdown() override;
	private:
		void DrawTexture(std::pair<const std::string, std::shared_ptr<VulkanTexture>>& texture);
	private:
		std::shared_ptr<SelectionManager> m_SelectionManager;
	};
}