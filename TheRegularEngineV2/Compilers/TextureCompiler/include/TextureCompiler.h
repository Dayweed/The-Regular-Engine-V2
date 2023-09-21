#pragma once
#include "Texture.h"
#include "TextureDescriptorFile.h"

namespace TRE
{
	class TextureCompiler
	{
	public:
		static TextureCompiler& Instance()
		{
			static TextureCompiler instance;
			return instance;
		}

		void Compile(const TextureDescriptorFile& descriptor);
		std::unique_ptr<Texture> GetTexture() { return std::move(m_Texture); }

	private:
		TextureCompiler() {};
		TextureCompiler(TextureCompiler const&) = delete;
		void operator=(TextureCompiler const&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unique_ptr<Texture> m_Texture;
	};
}