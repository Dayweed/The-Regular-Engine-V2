#include "TextureCompiler.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace TRE
{
	void TextureDescriptorFile::Write()
	{
		m_DescriptorFile << "Texture File Path:\n";
		m_TexturePath = m_AssetPath.substr(0, m_AssetPath.find_last_of("."));
		m_TexturePath += ".DDS";
		m_DescriptorFile << m_TexturePath << "\n\n";
		m_TextureName = m_TexturePath.substr(m_TexturePath.find_last_of("/") + 1);
		m_TextureName = m_TextureName.substr(0, m_TextureName.find_last_of("."));
		m_DescriptorFile << "Texture Name:\n";
		m_DescriptorFile << m_TextureName << "\n\n";
		m_DescriptorFile << "Width:\n";
		m_DescriptorFile << m_Width << "\n\n";
		m_DescriptorFile << "Height:\n";
		m_DescriptorFile << m_Height << "\n";
		m_DescriptorFile << "Check vulkan page for enums\n";
		m_DescriptorFile << "Texture Format:\n";
		m_DescriptorFile << m_Format << "\n\n";
		m_DescriptorFile << "Texture Filter:\n";
		m_DescriptorFile << m_Filter << "\n\n";
	}

	void TextureDescriptorFile::Read()
	{
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Texture File Path:")
		{
			std::getline(m_DescriptorFile, line);
			m_TexturePath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Name:")
		{
			std::getline(m_DescriptorFile, line);
			m_TextureName = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Name is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Width:")
		{
			std::getline(m_DescriptorFile, line);

			m_Width = std::stoi(line);
			
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Width missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Height:")
		{
			std::getline(m_DescriptorFile, line);

			m_Height = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Height missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Format:")
		{
			std::getline(m_DescriptorFile, line);

			m_Format = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Format missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Filter:")
		{
			std::getline(m_DescriptorFile, line);

			m_Filter = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Filter missing" << std::endl;
			return;
		}
	}

	void TextureCompiler::Compile(const TextureDescriptorFile& descriptor)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(descriptor.GetAssetPath().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
			return;
		}

		//Do compression here
		//crn_comp_params comp_params;
		//comp_params.m_width = texWidth;
		//comp_params.m_height = texHeight;
		//comp_params.m_file_type = cCRNFileTypeDDS;
		//comp_params.m_format = cCRNFmtDXT1;
		//comp_params.set_flag(cCRNCompFlagPerceptual, true);
		//comp_params.set_flag(cCRNCompFlagDXT1AForTransparency, true);
		//comp_params.set_flag(cCRNCompFlagHierarchical, true);
		//comp_params.m_quality_level = cCRNMinQualityLevel;
		//comp_params.m_dxt_quality = cCRNDXTQualitySuperFast;
		//comp_params.m_dxt_compressor_type = cCRNDXTCompressorRYG;
		//comp_params.m_pProgress_func = NULL;
		//comp_params.m_pProgress_func_data = NULL;
		//comp_params.m_num_helper_threads = 3;
		//comp_params.m_pImages[0][0] = reinterpret_cast<uint32_t*>(pixels);

		//crn_uint32 outputSize;
		//void* outputData = crn_compress(comp_params, outputSize);

		//crn_free_block(outputData);
		m_Texture = std::make_unique<Texture>();
		m_Texture->Data = reinterpret_cast<void*>(pixels);
		int len = std::min(descriptor.GetTextureName().length(), sizeof(m_Texture->Name));
		for (int i = 0; i < len; ++i)
			m_Texture->Name[i] = descriptor.GetTextureName()[i];
		m_Texture->Width = texWidth;
		m_Texture->Height = texHeight;
		m_Texture->Format = descriptor.GetFormat();
		m_Texture->Filter = descriptor.GetFilter();
	}
}