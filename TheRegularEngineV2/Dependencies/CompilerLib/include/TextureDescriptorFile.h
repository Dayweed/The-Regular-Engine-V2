#pragma once
#include "DescriptorFile.h"

namespace TRE
{
	class TextureDescriptorFile : public DescriptorFile
	{
	public:
		void SetTexturePath(const std::string& path) { m_TexturePath = path; }
		void SetTextureName(const std::string& name) { m_TextureName = name; }
		//void SetFormat(const int format) { m_Format = format; }
		//void SetFilter(const int filter) { m_Filter = filter; }
		void SetCompress(const bool compress) { m_Compress = compress; }
		void SetLinear(const bool linear) { m_Linear = linear; }
		void SetBCn(const int bcn) { m_BCn = bcn; }
		void SetsRGB(const bool RGB) { m_sRGB = RGB; }
		void SetTransparent(const bool transparent) { m_Transparent = transparent; }

		const std::string& GetTexturePath() const { return m_TexturePath; }
		const std::string& GetTextureName() const { return m_TextureName; }
		const int GetFormat() const { return m_Format; }
		const int GetFilter() const { return m_Filter; }
		const bool GetCompress() const { return m_Compress; }
		const bool GetLinear() const { return m_Linear; }
		const bool GetTransparent() const { return m_Transparent; }
		const int GetBCn() const { return m_BCn; }
		const bool GetsRGB() const { return m_sRGB; }
	protected:
		void Write() override;
		void Read() override;
	private:
		std::string m_TexturePath;
		std::string m_TextureName;
		int m_Format{ 43 }; //VK_FORMAT_R8G8B8A8_SRGB -- FOR SERIALIZATION ONLY
		int m_Filter{ 0 }; 	//NEAREST -- FOR SERIALIZATION ONLY
		int m_BCn{ 0 }; //BC1,3,5,7
		bool m_Compress{ 1 }; //NONE
		bool m_Linear{ 1 };
		bool m_Transparent{ 0 };
		bool m_sRGB{ 0 }; // if not RGB/RGBA THEN UNORM
	};
}