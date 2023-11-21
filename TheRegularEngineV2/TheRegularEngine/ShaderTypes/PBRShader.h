#pragma once
#include "Graphics/Shader.h"

namespace TRE
{
	class PBR
	{
	public:
		PBR(const std::string& hexHandle);
		static const ResourceHandle& GetDefaultMaterial();
		static const ResourceHandle& GetShaderHandle();
		static const ResourceHandle& GetDefaultAnimationMaterial();
		static const ResourceHandle& GetAnimationShaderHandle();

	private:
		static void CreateDefaultMaterial();
		static void CreateDefaultAnimationMaterial();

	private:
		static ResourceHandle m_DefaultMaterial;
		static ResourceHandle m_ShaderHandle;

		static ResourceHandle m_DefaultAnimationMaterial;
		static ResourceHandle m_AnimationShaderHandle;
	};
}