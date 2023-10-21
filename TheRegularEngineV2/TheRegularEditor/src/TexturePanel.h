#pragma once
#include "Panel.h"
#include "AssetSelector.h"

namespace TRE
{
	class TexturePanel : public Panel
	{
	public:
		TexturePanel(const std::shared_ptr<AssetSelector>& assetSelector);
		void Init() override;
		void Update() override;
		void Shutdown() override;

	private:
		std::shared_ptr<AssetSelector> m_AssetSelector;
	};
}