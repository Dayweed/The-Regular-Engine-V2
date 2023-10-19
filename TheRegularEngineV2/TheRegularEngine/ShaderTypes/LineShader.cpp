#include "pch.h"
#include "LineShader.h"
#include "Resource/ResourceManager.h"
#include "Graphics/ShaderReflection.h"

namespace TRE
{
	ResourceHandle Line::m_ShaderHandle{ 0 };

	Line::Line(const std::string& hexHandle)
	{
		m_ShaderHandle = Resource::GetGUIDFromHex(hexHandle);
		
		//std::unique_ptr<Shader> shader = ShaderCompiler::DeserializeReflectShader("../Resources/" + hexHandle + ".TREshader");
		std::unique_ptr<Shader> shader = ShaderCompiler::DeserializeReflectShader("../Resources/DebugDrawLine.TREshader");
		shader->SetHandle(m_ShaderHandle);
		ResourceManager::Instance().AddResource(std::move(shader));
	}

	const ResourceHandle& Line::GetShaderHandle()
	{
		return m_ShaderHandle;
	}
}