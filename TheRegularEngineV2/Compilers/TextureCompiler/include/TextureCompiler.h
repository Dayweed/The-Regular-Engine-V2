#pragma once
#include "Texture.h"
#include "DescriptorFile.h"

namespace TRE
{
	class TextureDescriptorFile : public DescriptorFile
	{
	public:
		void SetTexturePath(const std::string& path) { m_TexturePath = path; }
		void SetTextureName(const std::string& name) { m_TextureName = name; }
		void SetFormat(const int format) { m_Format = format; }
		void SetFilter(const int filter) { m_Filter = filter; }
		void SetCompress(const bool compress) { m_Compress = compress; }

		const std::string& GetTexturePath() const { return m_TexturePath; }
		const std::string& GetTextureName() const { return m_TextureName; }
		const int GetFormat() const { return m_Format; }
		const int GetFilter() const { return m_Filter; }
		const bool GetCompress() const { return m_Compress; }
	protected:
		void Write() override;
		void Read() override;
	private:
		std::string m_TexturePath;
		std::string m_TextureName;
		int m_Format{ 43 }; //VK_FORMAT_R8G8B8A8_SRGB 
		int m_Filter{ 0 }; //NEAREST
		bool m_Compress{ 0 }; //NONE
	};

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