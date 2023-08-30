#pragma once

namespace TRE
{
	class Shader
	{
		public:
			Shader();
			~Shader();


		private:
			VkShaderModule m_ShaderModule;


	};
}