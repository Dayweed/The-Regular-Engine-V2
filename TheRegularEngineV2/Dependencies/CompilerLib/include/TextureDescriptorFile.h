#pragma once
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
		void SetLinear(const bool linear) { m_Linear = linear; }
		void SetNormal(const bool normal) { m_NormalMap = normal; }

		const std::string& GetTexturePath() const { return m_TexturePath; }
		const std::string& GetTextureName() const { return m_TextureName; }
		const int GetFormat() const { return m_Format; }
		const int GetFilter() const { return m_Filter; }
		const bool GetCompress() const { return m_Compress; }
		const bool GetLinear() const { return m_Linear; }
		const bool GetNormalMap() const { return m_NormalMap; }
	protected:
		void Write() override;
		void Read() override;
	private:
		std::string m_TexturePath;
		std::string m_TextureName;
		int m_Format{ 43 }; //VK_FORMAT_R8G8B8A8_SRGB 
		int m_Filter{ 0 }; //NEAREST
		bool m_Compress{ 1 }; //NONE
		bool m_Linear{ 1 };
		bool m_NormalMap{ 0 };
	};
}