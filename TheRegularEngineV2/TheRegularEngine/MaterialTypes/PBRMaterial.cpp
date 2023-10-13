#include "pch.h"
#include "PBRMaterial.h"
#include "Resource/ResourceManager.h"
#include "Graphics/VulkanTexture.h"

namespace TRE
{
	ResourceHandle PBR::m_DefaultHandle{ 0 };

	const ResourceHandle& PBR::GetDefaultHandle()
	{
		if (m_DefaultHandle == 0)
		{
			CreateDefaultMaterial();
		}

		return m_DefaultHandle;
	}

	void PBR::CreateDefaultMaterial()
	{
		m_DefaultHandle = Resource::GenerateGUID();
		
		//Need change
		auto vertHandle = 3;
		auto fragHandle = 4;
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(vertHandle);
		auto FragShader = ResourceManager::Instance().GetResource<Shader>(fragHandle);
		std::unique_ptr<Material> defaultPBR = std::make_unique<Material>(VertShader, FragShader);
		defaultPBR->SetHandle(m_DefaultHandle);
		//For now since PBR takes in 4 textures, we will just use the same texture for all 4
		for(int i = 0; i < 4; i++)
			defaultPBR->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(VulkanTexture::GetDefaultTextureID()));

		ResourceManager::Instance().AddResource(std::move(defaultPBR));
	}
}