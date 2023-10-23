#include "pch.h"
#include "PBRShader.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Material.h"
#include "Graphics/ShaderReflection.h"

namespace TRE
{
	ResourceHandle PBR::m_DefaultMaterial{ 0 };
	ResourceHandle PBR::m_ShaderHandle{ 0 };

	PBR::PBR(const std::string& hexHandle)
	{
		m_ShaderHandle = Resource::GetGUIDFromHex(hexHandle);
		//Get shader path from shader descriptor file
		std::unique_ptr<Shader> shader = ShaderCompiler::DeserializeReflectShader("../Resources/" + hexHandle + ".TREshader");
		shader->SetHandle(m_ShaderHandle);
		ResourceManager::Instance().AddResource(std::move(shader));
	}

	const ResourceHandle& PBR::GetDefaultMaterial()
	{
		if (m_DefaultMaterial == 0)
		{
			CreateDefaultMaterial();
		}

		return m_DefaultMaterial;
	}

	const ResourceHandle& PBR::GetShaderHandle()
	{
		return m_ShaderHandle;
	}

	void PBR::CreateDefaultMaterial()
	{
		m_DefaultMaterial = Resource::GenerateGUID();
		
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(m_ShaderHandle);
		std::unique_ptr<Material> defaultPBR = std::make_unique<Material>(VertShader);
		defaultPBR->SetHandle(m_DefaultMaterial);
	
		ResourceManager::Instance().AddResource(std::move(defaultPBR));
	}
}