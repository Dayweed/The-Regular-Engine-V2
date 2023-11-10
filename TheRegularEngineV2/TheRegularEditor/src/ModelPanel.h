#pragma once
#include "Panel.h"
#include "AssetSelector.h"

namespace TRE
{
	class ModelPanel : public Panel
	{
	public:
		ModelPanel(const std::shared_ptr<AssetSelector>& assetSelector);
		void Init() override;
		void Update() override;
		void Shutdown() override;

		void OnAssetSelectEvent(const AssetSelectorEvent& event);
	private:
		std::shared_ptr<AssetSelector> m_AssetSelector;
		glm::vec3 m_Scale;
		glm::vec3 m_Rotation;
		glm::vec3 m_Translation;
		bool m_Optimize;

		std::string m_DescriptorFilePath;
		ResourceHandle m_ResourceHandle{ 0 };
		bool m_MeshSelected{ false };
	};
}