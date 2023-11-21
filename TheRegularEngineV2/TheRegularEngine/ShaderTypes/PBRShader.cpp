#include "pch.h"
#include "PBRShader.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Material.h"
#include "Graphics/ShaderReflection.h"

namespace TRE
{
	ResourceHandle PBR::m_DefaultMaterial{ 0 };
	ResourceHandle PBR::m_ShaderHandle{ 0 };

	ResourceHandle PBR::m_DefaultAnimationMaterial{0};
	ResourceHandle PBR::m_AnimationShaderHandle{0};

	PBR::PBR(const std::string& hexHandle)
	{
		m_ShaderHandle = Resource::GetGUIDFromHex(hexHandle);
		//Get shader path from shader descriptor file
		std::unique_ptr<Shader> shader = ShaderCompiler::DeserializeReflectShader("../Resources/PBR.TREshader");
		shader->SetHandle(m_ShaderHandle);
		ResourceManager::Instance().AddResource(std::move(shader));

		m_AnimationShaderHandle = 9;
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

	const ResourceHandle& PBR::GetAnimationShaderHandle()
	{
		return m_AnimationShaderHandle;
	}

	void PBR::CreateDefaultMaterial()
	{
		m_DefaultMaterial = Resource::GenerateGUID();
		
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(m_ShaderHandle);
		std::unique_ptr<Material> defaultPBR = std::make_unique<Material>(VertShader);
		defaultPBR->SetHandle(m_DefaultMaterial);
	
		ResourceManager::Instance().AddResource(std::move(defaultPBR));
	}

	void PBR::CreateDefaultAnimationMaterial()
	{
		m_DefaultAnimationMaterial = Resource::GenerateGUID();

		auto AnimShader = ResourceManager::Instance().GetResource<Shader>(m_AnimationShaderHandle);
		std::unique_ptr<Material> DefaultAnimationPBR = std::make_unique<Material>(AnimShader);
		DefaultAnimationPBR->SetHandle(m_DefaultAnimationMaterial);

		ResourceManager::Instance().AddResource(std::move(DefaultAnimationPBR));
	}

	const ResourceHandle& PBR::GetDefaultAnimationMaterial()
	{
		if (m_DefaultAnimationMaterial == 0)
		{
			CreateDefaultAnimationMaterial();
		}

		return m_DefaultAnimationMaterial;
	}
}