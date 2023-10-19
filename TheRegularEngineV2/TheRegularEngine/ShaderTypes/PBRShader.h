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

	private:
		static void CreateDefaultMaterial();

	private:
		static ResourceHandle m_DefaultMaterial;
		static ResourceHandle m_ShaderHandle;
	};
}