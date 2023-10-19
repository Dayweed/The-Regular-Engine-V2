#pragma once
#include "Graphics/Shader.h"

namespace TRE
{
	class Line
	{
	public:
		Line(const std::string& hexHandle);
		static const ResourceHandle& GetShaderHandle();
	private:
		static ResourceHandle m_ShaderHandle;
	};
}