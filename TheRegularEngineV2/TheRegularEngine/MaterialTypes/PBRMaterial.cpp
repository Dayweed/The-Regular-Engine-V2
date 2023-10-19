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
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(vertHandle);
		std::unique_ptr<Material> defaultPBR = std::make_unique<Material>(VertShader);
		defaultPBR->SetHandle(m_DefaultHandle);
	
		ResourceManager::Instance().AddResource(std::move(defaultPBR));
	}
}