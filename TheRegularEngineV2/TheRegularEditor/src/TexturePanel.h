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

		void OnAssetSelectEvent(const AssetSelectorEvent& event);

	private:
		std::shared_ptr<AssetSelector> m_AssetSelector;
		std::string m_HexHandle;
		bool m_Compress;
		bool m_Linear;
		std::uint32_t m_BCn;
		bool m_sRGB;
		bool m_Transparent;

		std::string m_DescriptorFilePath;
		ResourceHandle m_ResourceHandle{ 0 };
		bool m_TextureSelected{ false };
	};
}