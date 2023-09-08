#pragma once

namespace TRE
{
	class Shader
	{
		public:
			Shader() = default;
			~Shader() = default;
			std::string ReadGLSLToString(const std::string& filename);
			void LoadShader();
			int SkipBOM(std::istream& in);

		private:
			VkShaderModule m_ShaderModule;

	};
}