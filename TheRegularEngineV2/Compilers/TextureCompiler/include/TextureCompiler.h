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
		void SetWidth(const std::uint32_t width) { m_Width = width; }
		void SetHeight(const std::uint32_t height) { m_Height = height; }
		void SetFormat(const int format) { m_Format = format; }
		void SetFilter(const int filter) { m_Filter = filter; }

		const std::string& GetTexturePath() const { return m_TexturePath; }
		const std::string& GetTextureName() const { return m_TextureName; }
		const std::uint32_t GetWidth() const { return m_Width; }
		const std::uint32_t GetHeight() const { return m_Height; }
		const int GetFormat() const { return m_Format; }
		const int GetFilter() const { return m_Filter; }
	protected:
		void Write() override;
		void Read() override;
	private:
		std::string m_TexturePath;
		std::string m_TextureName;
		std::uint32_t m_Width{};
		std::uint32_t m_Height{};
		int m_Format{ 43 }; //VK_FORMAT_R8G8B8A8_SRGB 
		int m_Filter{ 0 }; //NEAREST
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